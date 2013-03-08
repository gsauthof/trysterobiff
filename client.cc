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




/*
 * Relevant RFCs:
 *
 * http://tools.ietf.org/html/rfc3501
 *     INTERNET MESSAGE ACCESS PROTOCOL - VERSION 4rev1  March 2003
 * http://tools.ietf.org/html/rfc2177
 *     IMAP4 IDLE command                                June 1997
 * http://tools.ietf.org/html/rfc2683
 *     IMAP4 Implementation Recommendations              September 1999
 * http://tools.ietf.org/html/rfc5322
 *     Internet Message Format                           October 2008
 * http://tools.ietf.org/html/rfc4234 
 *     Augmented BNF for Syntax Specifications: ABNF     October 2005
 *
 */

#include "client.hh"

#include <QSslSocket>
#include <QSettings>
#include <QTimer>

#include <cassert>
#include <iostream>

#include "name.hh"

using namespace std;

Client::Client()
    : state(DISCONNECTED), has_idle(false),
      socket(0), timer(0),
      port(0), timeout(30 * 1000),
      old_recent(0), fetched_rows(0), counter(0),
      preview_enabled(true), re_idle_intervall(28 * 60 * 1000),
      use_recent(true),
      has_recent(true),
      detect_gmail(true),
      update_always(false)
{
}

void Client::run()
{
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings s(IMAPBIFFNAME, IMAPBIFFNAME);
  host = s.value("host").toString();
  port = s.value("port").toInt();
  user = s.value("user").toString();
  pw = s.value("pw").toString();
  mbox = s.value("mbox", QVariant("INBOX")).toString();
  preview_enabled = s.value("preview", QVariant(true)).toBool();
  timeout = s.value("timeout", QVariant(30)).toInt()*1000;
  re_idle_intervall = s.value("re_idle", QVariant(28)).toInt()*60*1000;
  use_recent = s.value("use_recent", QVariant(true)).toBool();
  detect_gmail = s.value("detect_gmail", QVariant(true)).toBool();
  update_always = s.value("update_always", QVariant(false)).toBool();

  QTimer::singleShot(0, this, SLOT(setup()));
  exec();
}

void Client::setup()
{
  EMITDEBUG("START\n");
  socket = new QSslSocket();

  connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
      this, SLOT(so_changed(QAbstractSocket::SocketState)));
  connect(socket, SIGNAL(encrypted()), this, SLOT(so_encrypted()));
  connect(socket, SIGNAL(readyRead()), this, SLOT(so_read()));
  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
      this, SLOT(tcp_error(QAbstractSocket::SocketError)));
  connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
      this, SLOT(ssl_errors(QList<QSslError>)));

  connect(socket, SIGNAL(encrypted()), this, SIGNAL(connected()));
  connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));

  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(done()));

  socket->connectToHostEncrypted(host, port);
}

void Client::write_line(const QByteArray &a)
{
  int u = a.indexOf("login");
  if (u > -1) {
    QByteArray t(a);
    int x = t.indexOf(' ', u+6);
    if (x > -1)
      t.truncate(x+1);
    t.append("***");
    EMITDEBUG(">> " + QString::fromUtf8(t) + '\n');
  } else
    EMITDEBUG(">> " + QString::fromUtf8(a) + '\n');
  socket->write(a + "\r\n");
}

QByteArray Client::tag()
{
  ++counter;
  QByteArray a;
  a.append('A');
  a.append(QString::number(counter));
  return a;
}

#include <iostream>

void Client::so_changed(QAbstractSocket::SocketState i)
{
  switch (i) {
    case QAbstractSocket::ConnectedState :
      EMITDEBUG("Connected to: " + host + '\n');
      break;
    case QAbstractSocket::UnconnectedState :
      state = DISCONNECTED;
      idle_tag.clear();
      login_tag.clear();
      examine_tag.clear();
      search_tag.clear();
      fetch_tag.clear();
      query.clear();
      break;
    default:
      break;
  }
}

void Client::so_encrypted()
{
  EMITDEBUG("Secured channel.\n");
  state = CONNECTED;
}

void Client::ssl_errors(const QList<QSslError> &errors)
{
  foreach(const QSslError &e, errors)
    emit error(e.errorString());
}

void Client::tcp_error(QAbstractSocket::SocketError e)
{
  emit error(socket->errorString() + " == " + QString::number(int(e)));
}

void Client::so_read()
{
  if (!socket->canReadLine()) {
    EMITDEBUG("Line not complete. Buffer more.\n");
    return;
  }
  while (socket->canReadLine()) {
    QByteArray a = socket->readLine();
    EMITDEBUG("<< " + QString::fromUtf8(a) + '\n');
    parse(a);
  }
}

bool Client::tag_ok(const QByteArray &a, QByteArray &t)
{
  if (t.isEmpty()) {
    error_close("Empty tag.");
    return false;
  }
  if (a.startsWith(t + " OK ")) {
    if (!t.startsWith('*'))
      t.clear();
    return true;
  }
  return false;
}

bool Client::tag_ok(const QByteArray &a, QByteArray &t, State s)
{
  if (tag_ok(a, t)) {
    state = s;
    return true;
  }
  return false;
}

bool Client::check_gmail(const QByteArray &u)
{
  if (detect_gmail && u.contains(" GIMAP ")) {
    EMITDEBUG("Connected to Gmail-Server - using UNSEEN instead of RECENT");
    has_recent = false;
    update_always = true;
    return true;
  }
  return false;
}

// #include <iostream>

void Client::parse(const QByteArray &a)
{
  QByteArray u = a.toUpper();
  //std::cerr << "XXX (state " << state << ") " << u.constData() << '\n';
  if (parse_recent(u))
    return;
  if (parse_error(u))
    return;
  QByteArray untag("*");
  switch (state) {
    case CONNECTED :
      has_idle = false;
      if (tag_ok(u, untag, PRELOGIN)) {
        check_gmail(u);
        QTimer::singleShot(0, this, SLOT(login()));
      }
      break;
    case LOGGINGIN:
      check_capabilities(u);
      if (tag_ok(u, login_tag, PREEXAMINE)) {
        check_capabilities(u);
        if (has_idle)
          QTimer::singleShot(0, this, SLOT(examine()));
      }
      break;
    case EXAMING:
      if (tag_ok(u, examine_tag, PREIDLE))
        QTimer::singleShot(0, this, SLOT(idle()));
      break;
    case STARTINGIDLE:
      parse_idle_ok(u);
      break;
    case IDLING:
      if (tag_ok(u, idle_tag, POSTIDLE)) {
        timer->stop();
        if (old_recent)
          QTimer::singleShot(0, this, SLOT(search()));
        else
          QTimer::singleShot(0, this, SLOT(idle()));
      }
      break;
    case SEARCHING:
      parse_search_res(u);
      if (tag_ok(u, search_tag, PREFETCH))
        QTimer::singleShot(0, this, SLOT(fetch()));
      break;
    case FETCHING:
      parse_header_field(a);
      parse_header_end(u);
      if (tag_ok(u, fetch_tag, STARTINGIDLE)) {
        if (fetched_rows) {
          emit new_messages(fetched_rows);
          if (preview_enabled)
            emit new_headers(headers);
        }
        headers.clear();
        QTimer::singleShot(0, this, SLOT(idle()));
      }
      break;
    default:
      break;
  }
}

bool Client::parse_error(const QByteArray &u)
{
  int x = u.indexOf(' ');
  if (x<0)
    return false;
  if (u.mid(x+1).startsWith("BAD ") ||
      u.mid(x+1).startsWith("NO ")) {
    error_close(u);
    return true;
  }
  return false;
}

bool Client::parse_recent(const QByteArray &u)
{
  const char *status_clause = 0;
  if (has_recent)
    status_clause = "RECENT";
  else
    status_clause = "EXISTS";
  if (!u.startsWith("*"))
    return false;
  int x = u.indexOf(' ');
  if (x<1)
    return false;
  int y = u.indexOf(' ', x+1);
  if (y<0)
    return false;
  if (!u.mid(y+1).startsWith(status_clause))
    return false;
  bool b = true;
  int msg = u.mid(x+1, y-x-1).toInt(&b);
  //assert(msg >= 0);
  EMITDEBUG("# msgs: " + QString::number(msg) + '\n');
  if (!b)
    emit error("RECENT/EXISTS parse error");
  EMITDEBUG("Minutes since last status push: "
      + QString::number(double(time.restart())/1000.0/60));
  if (preview_enabled || !has_recent) {
    if (state == IDLING) {
      old_recent = msg;
      if (!old_recent)
          emit new_messages(size_t(msg));
      done();
    } else { // state == EXAMING
      if (update_always || old_recent != size_t(msg)) {
        old_recent = msg;
        search();
      }
    }
  } else {
    old_recent = msg;
    emit new_messages(size_t(msg));
  }
  return true;
}

bool Client::check_capabilities(const QByteArray &u)
{
  // we need to parse two styles:
  // 1.) A1 OK [CAPABILITY ... IDLE ...
  //     -> as answer to login
  // 2.) * CAPABILITY ... IDLE ...
  //     -> untagged answer (e.g. gmail servers)

  if (!u.contains("CAPABILITY"))
    return false;
  if (!u.contains(" IDLE")) {
    socket->disconnectFromHost();
    EMITDEBUG("server does NOT have CAPABILITY IDLE");
    return false;
  }
  EMITDEBUG("server DOES have CAPABILITY IDLE");
  has_idle = true;
  return true;
}

bool Client::parse_idle_ok(const QByteArray &u)
{
  if (u.startsWith("+ IDLING")) {
    state = IDLING;
    return true;
  }
  return false;
}

void Client::parse_search_res(const QByteArray &u)
{
  if (!u.startsWith("* SEARCH "))
    return;
  if (!query.isEmpty())
    query.append(' ');
  query.append(u.mid(9).simplified());
  query.replace(' ', ',');
}

void Client::parse_header_field(const QByteArray &a)
{
  QByteArray u = a.toUpper();
  if (u.startsWith("SUBJECT: "))
    subject = a.mid(9).trimmed();
  if (u.startsWith("FROM: "))
    from = a.mid(6).trimmed();
  if (u.startsWith("DATE: "))
    date = a.mid(6).trimmed();
}

void Client::parse_header_end(const QByteArray &u)
{ 
  if (u.startsWith(")")) {
      headers.append(subject + " - " + from + " (" + date + ")\n" );
      subject.clear();
      from.clear();
      date.clear();
      ++fetched_rows;
  }
}

void Client::login()
{
  assert(login_tag.isEmpty());
  login_tag = tag();
  state = LOGGINGIN;
  time.start();
  write_line(login_tag + " login " + user.toUtf8() + " " + pw.toUtf8());
}

void Client::examine()
{
  assert(examine_tag.isEmpty());
  examine_tag = tag();
  state = EXAMING;
  write_line(examine_tag + " examine " + mbox.toUtf8());
}

void Client::idle()
{
  assert(idle_tag.isEmpty());
  idle_tag = tag();
  state = STARTINGIDLE;
  timer->start(re_idle_intervall);
  write_line(idle_tag + " idle");
}

void Client::done()
{
  if (socket->state() != QAbstractSocket::ConnectedState)
    return;
  if (state != IDLING)
    return;
  write_line("done");
}

void Client::search()
{
  //if (!old_recent)
  //  return;
  if (!search_tag.isEmpty() || !fetch_tag.isEmpty()) {
    emit error("Last search is still active!");
    return;
  }
  assert(search_tag.isEmpty());
  search_tag = tag();
  state = SEARCHING;
  if (has_recent)
    write_line(search_tag + " search RECENT");
  else
    write_line(search_tag + " search UNSEEN");
}

void Client::fetch()
{
  state = FETCHING;
  if (query.isEmpty()) {
    //error_close("Empty query.");
    fetched_rows = 0;
    emit new_messages(fetched_rows);
    EMITDEBUG("Search returned nothing.");
    QTimer::singleShot(0, this, SLOT(idle()));
    return;
  }
  assert(fetch_tag.isEmpty());
  fetch_tag = tag();
  fetched_rows = 0;
  write_line(fetch_tag + " fetch " + query
      + " (internaldate flags body[header.fields (date from subject)])");
  query.clear();
}


Client::~Client()
{
  delete timer;
  delete socket;
  quit();
  wait();
}

void Client::preview_toggle(bool b)
{
  QSettings s(IMAPBIFFNAME, IMAPBIFFNAME);
  s.setValue("preview", QVariant(b));
  preview_enabled = b;
  if (b && old_recent)
    search();
}

void Client::do_connect()
{
  if (!socket || socket->state() != QAbstractSocket::UnconnectedState) {
    emit error("Cannot connect.");
    return;
  }
  socket->connectToHostEncrypted(host, port);
  has_recent = use_recent;
}

#include <iostream>

void Client::do_disconnect()
{
  if (!socket || socket->state() == QAbstractSocket::UnconnectedState) {
    emit error("Cannot disconnect.");
    return;
  }
  socket->disconnectFromHost();
}

void Client::error_close(const QString &s)
{
  emit error(s);
  emit error("Disconnected because of errors.");
  if (!socket)
    return;
  if (socket->state() == QAbstractSocket::ClosingState ||
      socket->state() == QAbstractSocket::UnconnectedState)
    EMITDEBUG("error close, but closing or closed");
  socket->disconnectFromHost();
}


