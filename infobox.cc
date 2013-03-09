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


#include "infobox.hh"

#include <QTextEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QShortcut>

Infobox::Infobox(QWidget *parent)
  : QDialog(parent), textbox(0), saved(false)
{
  QGridLayout *layout = new QGridLayout(this);
  textbox = new QTextEdit(this);
  textbox->setReadOnly(true);
  layout->addWidget(textbox);
  QPushButton *hide = new QPushButton(tr("&Hide"), this);
  connect(hide, SIGNAL(clicked()), this, SLOT(hide()));

  QShortcut *c = new QShortcut(QKeySequence(tr("Ctrl+W", "Hide dialog")), this);
  connect(c, SIGNAL(activated()), this, SLOT(hide()));

  layout->addWidget(hide);
  setLayout(layout);
  setWindowTitle(tr("Messages"));
}

void Infobox::add_line(const QString &a)
{
  QTextCursor c = textbox->textCursor();
  c.movePosition(QTextCursor::End);
  int n = c.position();
  c.insertText(a);
  if (!a.endsWith('\n'))
    c.insertText("\n");
  if (n > 1024) {
    c.movePosition(QTextCursor::Start);
    c.select(QTextCursor::LineUnderCursor);
    c.removeSelectedText();
  }
  textbox->setTextCursor(c);
}

void Infobox::setVisible(bool b)
{
  if (saved)
    setGeometry(old_geo);
  QDialog::setVisible(b);
}

void Infobox::save_geo()
{
  old_geo = geometry();
  saved = true;
  emit hidden();
}

void Infobox::closeEvent(QCloseEvent *)
{
  // When is this exactly called?
  save_geo();
}


void Infobox::hideEvent(QHideEvent *)
{
  save_geo();
}
