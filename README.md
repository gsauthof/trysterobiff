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

    $ cp trysterobiff.conf $HOME/.config/trysterobiff.conf

Restrict the read access:

    $ chmod 600 $HOME/.config/trysterobiff.conf

And adjust the default settings (hostname etc.):

    $ vim $HOME/.config/trysterobiff.conf


## SSL ##

Trysterobiff only supports IMAP over SSL/TLS and only accepts certificates that
can be checked against a known CA root certificate. It is possible to configure
an additional CA root certificate, in case that it is not available systemwide.

If you have problems to identify the root SSL-certficate the server certificate
was signed with, check out the notes at the end of:

https://bitbucket.org/gsauthof/mailcp/overview 


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

You can explicitly configure this behavior for other IMAP daemons, too or you can
even disable the Gmail autodetection (cf. trysterobiff.conf example).

When connecting to non-Gmail-IMAPDs, trysterobiff searches by default for
RECENT messages (RECENT messages are a subset of the UNSEEN ones).


## Multiple Account ##

If you want to use multiple Trysterobiff instances (for multiple IMAP
accounts), just set the HOME environment variable like this:

    $ HOME=/home/juser/accountb ./trysterobiff

Assuming that /home/juser/accountb/.config/trysterobiff.conf is setup.

Perhaps it makes sense to add an option to Trysterobiff for user defined config file names under $HOME/.conf/.


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

