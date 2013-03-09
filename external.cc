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

#include "external.hh"

#include <QProcess>
#include <QSettings>


External::External()
  : p(0)
{
  QSettings s;
  QVariant v = s.value("external_cmd");
  if (!v.isValid())
    return;
  cmd = v.toString();
  p = new QProcess(this);

}

void External::new_messages(size_t i)
{
  if (!i)
    return;
  if (!p || p->state() != QProcess::NotRunning)
    return;
  QString t(cmd);
  t.replace("%d", QString::number(i));
  p->start(t);
}


