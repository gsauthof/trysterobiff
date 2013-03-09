/* {{{

    This file is part of trysterobiff -
      a cross-plattform non-polling IMAP new-mail systray notifier.

    Copyright (C) 2013  Georg Sauthoff
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

#include <decode.hh>

#include <QTextCodec>
#include <cctype>

#include <QDebug>
#define QT_NO_DEBUG_OUTPUT

using namespace std;

// MIME (Multipurpose Internet Mail Extensions) Part Three:
//               Message Header Extensions for Non-ASCII Text
// http://tools.ietf.org/html/rfc2047
namespace Decode {

  static bool check_encoded(const QByteArray &s)
  {
    for (int i = 0; i<s.size(); ++i)
    {
      char c = s.at(i);
      if (c <= 0x20 || c > 0x7e) {
        qDebug() << "Forbidden character at pos " << i << ": " << int(c);
        return false;
      }
    }
    return true;
  }

  static void qdecode(const QByteArray &src, QByteArray &dest)
  {
    QByteArray src2(src);
    src2.replace('_', ' ');
    qDebug() << "replaced _: " << src2;
    dest.clear();
    dest.reserve(src.size());
    dest = QByteArray::fromPercentEncoding(src2, '=');
    qDebug() << "percent decoded: " << dest;
  }

  void words(const QByteArray &src, QByteArray &dst)
  {
    // RFC talks about max line size of 76 chars
    // the maximal size of an encoded word is 75 chars
    if (src.size() > 1024) {
      dst = src;
      return;
    }
    int last = 0;
    for (;;) {
      int start = src.indexOf("=?", last);
      if (start == -1)
        break;
      int mid = src.indexOf('?', start+2);
      if (mid == -1)
        break;
      char encoding = toupper(src.at(mid+1));
      qDebug() << "encoding: " << encoding;
      if (encoding != 'Q' && encoding != 'B') {
        dst.append(src.mid(last, mid-last));
        last = mid;
        continue;
      }
      int end = src.indexOf("?=", mid+3);
      if (end == -1)
        break;
      QByteArray charset(src.mid(start+2, mid-start-2));
      qDebug() << "charset: " << charset;
      QTextCodec *codec = QTextCodec::codecForName(charset);
      if (!codec) {
        qDebug() << "Unknown charset";
        dst.append(src.mid(last, end+2-last));
        last = end+2;
        continue;
      }
      QByteArray encoded_text(src.mid(mid+3, end-mid-3));
      qDebug() << "Encoded text: " << encoded_text;
      if (!check_encoded(encoded_text)) {
        dst.append(src.mid(last, end+2-last));
        last = end+2;
        continue;
      }
      QByteArray decoded_text;
      switch (encoding) {
        case 'Q' : qdecode(encoded_text, decoded_text);                 break;
        case 'B' : decoded_text = QByteArray::fromBase64(encoded_text); break;
      }
      qDebug() << "Decoded_text: " << decoded_text;
      qDebug() << "QString: " << codec->toUnicode(decoded_text);
      QByteArray utf8_text = codec->toUnicode(decoded_text).toUtf8();
      qDebug() << "UTF8_text: " << utf8_text;
      dst.append(utf8_text);
      last = end+2;
    }
    dst.append(src.mid(last));
  }

}

#if 0

#include <stdio.h>

int main(int argc, char **argv)
{
  // not needed here
  // QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
  // needed for QByteArray::fromPercentEncoding() and QByteArray::fromBase64()
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
  while (!feof(stdin))
  {
    char buffer[1024] = {0};
    char *ret = fgets(buffer, 1024, stdin);
    if (!ret)
      break;
    QByteArray in(buffer);
    qDebug() << "Read: " <<  in;
    QByteArray out;
    Decode::words(in, out);
    qDebug() << "Decoded: " <<  out;
  }
  return 0;
}

#endif


