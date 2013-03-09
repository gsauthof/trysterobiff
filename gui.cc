/* {{{

    This file is part of trysterobiff -
      a cross-plattform non-polling IMAP new-mail systray notifier.

    Copyright (C) 2011  Georg Sauthoff
         email: mail@georg.so or gsauthof@sdf.lonestar.org

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

}}} */



#include <QApplication>
#include <QSystemTrayIcon>
#include <QSslSocket>
#include <QTimer>
#include <QSettings>
#include <QTextCodec>
#include <QDir>

#include "tray.hh"
#include "client.hh"
#include "external.hh"

#include "dummy.hh"

#include <iostream>

#include "name.hh"

using namespace std;

void help(const char *prog)
{
  cerr << "Call: " << prog <<
    "(Option)*\n\n"
    "where Option is one of:\n"
    "\n"
   //--------------------------------------------------------------------------------
    "  --help            this screen\n"
    "  --settings DIR    read trysterobiff.conf from DIR\n"
    "  --debug           print diagnostic output to stderr\n\n";
   //--------------------------------------------------------------------------------
}


struct Options {
  bool debug;
  QString settings_path;
  Options()
    :
      debug(false)
  {
  }
  void parse_args(int argc, char **argv)
  {
    for (int i = 1; i<argc; ++i)
      if (!strcmp(argv[i], "--debug"))
        debug = true;
      else if (!strcmp(argv[i], "--help")) {
        help(argv[0]);
        exit(0);
      } else if (!strcmp(argv[i], "--settings")) {
        ++i;
        if (i>=argc) {
          cerr << "Missing argument to --settings\n";
          exit(1);
        }
        settings_path = argv[i];
        QDir dir(settings_path);
        if (!dir.exists()) {
          cerr << "Settings directory '" << argv[i] << "' does not exist\n";
          exit(1);
        }
      } else {
        cerr << "Unknown option: " << argv[i] << '\n';
        help(argv[0]);
        exit(1);
      }
  }
  Options(int argc, char **argv)
    :
      debug(false)
  {
    parse_args(argc, argv);
  }

};


static void setup_settings(const Options &opts)
{
  QCoreApplication::setOrganizationName(IMAPBIFFNAME);
  QCoreApplication::setApplicationName(IMAPBIFFNAME);

  QSettings::setDefaultFormat(QSettings::IniFormat);
  if (!opts.settings_path.isEmpty()) {
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
        opts.settings_path);
  }
  QSettings s;
  if (!s.value("host").isValid()) {
    std::cerr << "Config file is missing. Copy example file to ";
    if (opts.settings_path.isEmpty())
      cerr << "$HOME/.config";
    else
      cerr << opts.settings_path.toUtf8().constData();
    cerr << "/" IMAPBIFFNAME ".conf , \n"
      " chmod 600 it and adjust the settings.\n";
    exit(6);
  }

  QString cert = s.value("cert").toString();
  if (cert != "") {
    bool b = QSslSocket::addDefaultCaCertificates(cert);
    if (!b) {
      std::cerr << "Could not load additional root certificate: "
        << cert.toUtf8().constData() << '\n';
      exit(7);
    }
  }
}


int main(int argc, char **argv)
{
  Options opts(argc, argv);
  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(false);

  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    std::cerr << "Could not find a System Tray.\n";
    return 42;
  }
  if (!QSslSocket::supportsSsl()) {
    std::cerr << "WTF?!? Qt lib does not support SSL ...\n";
    return 23;
  }

  setup_settings(opts);

  Tray t;
  Client c;
  External e;

#ifndef NOIMAPDEBUG
  QObject::connect(&c, SIGNAL(debug(const QString&)), &t, SLOT(debug(const QString&)));
  if (opts.debug)
  {
    Dummy *d = new Dummy();
    QObject::connect(&c, SIGNAL(debug(const QString&)), d, SLOT(error(const QString&)));
    QObject::connect(&c, SIGNAL(error(const QString&)), d, SLOT(error(const QString&)));
    QObject::connect(&c, SIGNAL(new_messages(size_t)),  d, SLOT(new_messages(size_t)));
  }
#endif
  QObject::connect(&c, SIGNAL(error(const QString&)), &t, SLOT(error(const QString&)));
  QObject::connect(&c, SIGNAL(new_messages(size_t)), &t, SLOT(new_messages(size_t)));
  QObject::connect(&c, SIGNAL(new_headers(const QByteArray&)), &t, SLOT(new_headers(const QByteArray&)));
  QObject::connect(&c, SIGNAL(connected()), &t, SLOT(connected()));
  QObject::connect(&c, SIGNAL(disconnected()), &t, SLOT(disconnected()));

  QObject::connect(&t, SIGNAL(preview_toggled(bool)), &c, SLOT(preview_toggle(bool)));
  QObject::connect(&t, SIGNAL(connect_requested()), &c, SLOT(do_connect()));
  QObject::connect(&t, SIGNAL(disconnect_requested()), &c, SLOT(do_disconnect()));

  QObject::connect(&c, SIGNAL(new_messages(size_t)), &e, SLOT(new_messages(size_t)));

  c.start();
  return app.exec();
}

