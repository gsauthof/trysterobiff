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



#include <QCoreApplication>
#include <QSslSocket>
#include <QTimer>
#include <QSettings>

#include "client.hh"
#include "dummy.hh"

#include <iostream>

#include "name.hh"

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  if (!QSslSocket::supportsSsl()) {
    std::cerr << "WTF?!? Qt lib does not support SSL ...\n";
    return 23;
  }
  QCoreApplication::setOrganizationName(IMAPBIFFNAME);
  QCoreApplication::setApplicationName(IMAPBIFFNAME);
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings s;
  QString cert = s.value("cert").toString();
  if (cert != "") {
    bool b = QSslSocket::addDefaultCaCertificates(cert);
    if (!b)
      std::cerr << "Could not load additional root certificate: " << cert.toUtf8().constData() << '\n';
  }

  Client c;

  Dummy d;
  QObject::connect(&c, SIGNAL(debug(const QString&)), &d, SLOT(error(const QString&)));
  QObject::connect(&c, SIGNAL(error(const QString&)), &d, SLOT(error(const QString&)));
  QObject::connect(&c, SIGNAL(new_messages(size_t)), &d, SLOT(new_messages(size_t)));
  QObject::connect(&c, SIGNAL(new_headers(const QByteArray&)), &d, SLOT(new_headers(const QByteArray&)));

  c.start();

  //QTimer::singleShot(0, c, SLOT(start()));
  return app.exec();
}
