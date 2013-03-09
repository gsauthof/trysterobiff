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


#ifndef CLIENT_HH
#define CLIENT_HH

#include <QObject>

#include <QThread>

class QSslSocket;
class QSslError;
class QTimer;

class Slave;

#include <QSslSocket>

#include <QTime>


#ifdef NOIMAPDEBUG
#define EMITDEBUG(A) do {} while(0)
#else
#define EMITDEBUG(A) emit debug(A)
#endif


class Client : public QThread {
  Q_OBJECT
  public:
    Client();
    ~Client();
  public slots:
    void run();
    void preview_toggle(bool);
    void do_connect();
    void do_disconnect();
  signals:
    void new_messages(size_t);
    void new_messages_slave(size_t);
    void new_headers(const QByteArray&);
    void error(const QString&);
    void debug(const QString&);

    void connected();
    void disconnected();

  private:
    enum State {
      DISCONNECTED,
      CONNECTED,
      PRELOGIN,
      LOGGINGIN,
      PREEXAMINE,
      EXAMING,
      PREIDLE,
      STARTINGIDLE,
      IDLING,
      POSTIDLE,
      SEARCHING,
      PREFETCH,
      FETCHING
    };

    QTime time;
    QDateTime last_connect;

    State state;

    bool has_idle;

    QSslSocket *socket;
    QTimer *timer;
    QString host, user, pw, mbox;
    int port;
    int timeout;
    size_t old_recent;
    size_t fetched_rows;
    size_t counter;
    QByteArray login_tag, examine_tag, idle_tag, search_tag, fetch_tag, query,
               subject, from, date, headers;
    bool preview_enabled;
    size_t re_idle_intervall;
    bool use_recent;
    bool has_recent;
    bool detect_gmail;
    bool update_always;
    bool auto_reconnect;

    void write_line(const QByteArray &);
    void error_close(const QString &);

    QByteArray tag();
    bool tag_ok(const QByteArray &a, QByteArray &t);
    bool tag_ok(const QByteArray &a, QByteArray &t, State s);

    void parse(const QByteArray &a);
    bool parse_error(const QByteArray &u);
    bool parse_recent(const QByteArray &);
    bool check_capabilities(const QByteArray &u);
    bool check_gmail(const QByteArray &u);
    bool parse_idle_ok(const QByteArray &u);
    void parse_search_res(const QByteArray &u);
    void parse_header_field(const QByteArray &a);
    void parse_header_end(const QByteArray &u);

    void reconnect();

  private slots:
    void setup();

    void login();
    void examine();
    void idle();
    void done();
    void search();
    void fetch();

    void so_changed(QAbstractSocket::SocketState);
    void so_encrypted();
    void so_read();
    void ssl_errors(const QList<QSslError> &);
    void tcp_error(QAbstractSocket::SocketError);

};


#endif
