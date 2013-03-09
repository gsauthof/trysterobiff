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



#include "dummy.hh"

#include <iostream>

void Dummy::error(const QString &e)
{
  std::cerr << "MSG: " << e.toUtf8().constData() << '\n';
}

void Dummy::new_messages(size_t n)
{
  std::cerr << "New Messages: " << n << '\n';
}

void Dummy::new_headers(const QByteArray &a)
{
  std::cerr << "XXX\n";
  std::cerr << a.constData();
  std::cerr << "XXX\n";
}
