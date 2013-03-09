Trysterobiff is a cross-plattform non-polling IMAP new-mail systray notifier.

Instead of polling it uses the IDLE extension of IMAP. With the IDLE extension
an IMAP server immediately sends mailbox status updates to the client. Using
this mechanism you really get notified of new mail as fast as possible.

In comparison to that, with a polling client there is a time-window where a new
mail arrives at the server and the next scheduled polling.

Because of this property Trysterobiff is also called Workaholic Biff.

Besides visual notifications it supports executing external commands on new
mail arrival.

Trysterobiff is written in C++ and uses the Qt library (version 4.x). It is
licenced under the GPL v3+.

2011-01-08, 2013-03-05


## Contact ##

I appreciate feedback and comments:

    mail@georg.so
    gsauthof@sdf.lonestar.org


## Install ##

You need to have Qt 4 installed, version >= 4.3 or 4.4 probably. It was tested
with version 4.6.

Generate the Makefile via:

    $ qmake gui.pro

Or

    $ qmake "CFG=release" gui.pro

(see gui.pro for more alternatives)

And then compile it via:

    $ make

The resulting binary can be called via:

    $ ./trysterobiff &

## Config ##

The repository contains a commented config file template, which includes all
available options.

Copy it to the right location:

    $ cp trysterobiff.ini $HOME/.config/trysterobiff.ini

Restrict the read access:

    $ chmod 600 $HOME/.config/trysterobiff.ini

And adjust the default settings (hostname etc.):

    $ vim $HOME/.config/trysterobiff.ini


## SSL ##

Trysterobiff only supports IMAP over SSL/TLS and only accepts certificates that
can be checked against a known CA root certificate. It is possible to configure
an additional CA root certificate, in case that it is not available systemwide.

If you have problems to identify the root SSL-certficate the server certificate
was signed with, check out the notes at the end of:

https://bitbucket.org/gsauthof/mailcp/overview 


## Mouse control ##

Single-clicks on tray icon:

- left   - re-display preview bubble
- middle - re-connect to server (may take 1-2 seconds)
- right  - show menu


## Gmail ##

Gmail IMAP access has following [limitation][gmaillim], which matters to
trysterobiff:

> Gmail IMAP1 is a fairly complete implementation of IMAP, but the
> following features are currently unsupported:
>
>   - \Recent flags on messages. [..]'

[gmaillim]: http://support.google.com/mail/bin/answer.py?hl=en&answer=78761


Trysterobiff detects this and on connecting to a GMail-IMAPD, it will
automatically use search-for-UNSEEN messages.

You can explicitly configure this behavior for other IMAP daemons, too, or you
can even disable the Gmail autodetection (cf. trysterobiff.conf example).

When connecting to non-Gmail-IMAPDs, Trysterobiff searches by default for
RECENT messages (RECENT messages are a subset of the UNSEEN ones).

With that work-around Trysterobiff behaves like this when connected to a gmail-imapd:

- When reading messages in the gmail web-client, gmail does NOT immediately
  push the status change (i.e. read -> unread attribute change of a message)
  over imap. But gmail pushes every 5 minutes a regular
  status update. On that update Trysterobiff has the chance to update
  notifications. Thus, in the best, worst or average case this update is immediate,
  5 or 2.5 minutes late.
- When using the android gmail app, you have to hit the sync button (or wait
  for an automated sync) until changes are propagated via IMAP. But the sync
  then immediately pushes a new status over imap.
- On incoming new mail gmail nearly immediately pushes a status update over IMAP.
- When deleting messages (via a concurrent imap connection) gmail also pushes a new
  status immediately.

Thus, only the first point is a real limitation of the workaround. 'Standard'
IMAP servers that has RECENT flag support don't have that limitation, of
course.

## Multiple Account ##

If you want to use multiple Trysterobiff instances (for multiple IMAP
accounts), just specify an alternative config path:

    $ trysterobiff --settings $HOME/.config/accountb

In the above call $HOME/.config/accountb/trysterobiff.ini is used as config file.

Alternatively, you can set the XDG_CONFIG_HOME environment variable, e.g.:

    $ XDG_CONFIGHOME=$HOME/.config/accountb trysterobiff

Or you can even change your HOME environment variable:

    $ HOME=/home/juser/accountb ./trysterobiff

(for using /home/juser/accountb/.config/trysterobiff.ini)


## Tested IMAP-Servers ##

* Cyrus IMAP v2.3.13
* [Dovecot][dove] 1.2.9
* [Gmail][gmail] (2013-03)

## Misc ##

* Note that the Qt API documentation specifies that the bubble display feature
  is not available on Mac OSX ...

* The included icons are from the [tango project][tango]. They are Public Domain.


[tango]: http://tango.freedesktop.org/Tango_Desktop_Project
[dove]: http://www.dovecot.org/
[gmail]: http://en.wikipedia.org/wiki/Gmail

