/* {{{

    This file is part of trysterobiff -
      a cross-plattform non-polling IMAP new-mail systray notifier.

    Copyright (C) 2011  Georg Sauthoff
         email: gsauthof@techfak.uni-bielefeld.de or gsauthof@sdf.lonestar.org

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


#ifndef TRAY_HH
#define TRAY_HH

#include <QObject>
#include <QIcon>
#include <QSystemTrayIcon>

class QAction;

class Infobox;

class Tray : public QObject {
  Q_OBJECT
  public:
    Tray();
    ~Tray();
  public slots:
    void action(QSystemTrayIcon::ActivationReason);

    void error(const QString&);
    void debug(const QString&);
    void new_messages(size_t);
    void new_headers(const QByteArray&);

    void connected();
    void disconnected();
  signals:
    void preview_toggled(bool);
    void connect_requested();
    void disconnect_requested();

  private slots:
    void preview_toggle(bool);
    void about();
  private:
    QSystemTrayIcon *tray;
    QAction *con, *discon, *action_info;
    Infobox *infobox;
    QIcon icon_newmail, icon_normal, icon_error, icon_disconnected;
    size_t new_msg;
    bool show_preview;
    size_t preview_time;
    QByteArray headers;
    bool pre_reconnect;
    void reconnect();
    void add_info(const QString &);
    void setup_menu();
    void setup_infobox();
    void show_message();
};

#endif
