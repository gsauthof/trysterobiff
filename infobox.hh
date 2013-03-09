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




#ifndef INFOBOX_HH
#define INFOBOX_HH

#include <QDialog>
#include <QRect>
class QTextEdit;

class Infobox : public QDialog {
  Q_OBJECT
  public:
    Infobox(QWidget *parent);
  public slots:
    void add_line(const QString &);
    void setVisible(bool);
  signals:
    void hidden();

  protected:
    void closeEvent(QCloseEvent *e);
    void hideEvent(QHideEvent *e);
  private:
    QTextEdit *textbox;
    bool saved;
    QRect old_geo;

    void save_geo();
};


#endif
