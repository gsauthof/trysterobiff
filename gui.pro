TEMPLATE = app
QT += network svg

# To disable debug code use:
#     qmake "CFG=release" gui.pro
# or
#     qmake "CFG=nodebug" gui.pro
CONFIG += debug

contains(CFG, release) {
  message (Release build)
  CONFIG -= debug
}
contains(CFG, nodebug) {
  message (Release build without IMAP debug code.)
  DEFINES += NOIMAPDEBUG
  CONFIG -= debug
  # is not expensive:
  # DEFINES += NDEBUG
}

message (Using Qt CONFIG: $$CONFIG)

TARGET = trysterobiff

HEADERS += tray.hh client.hh name.hh external.hh infobox.hh
SOURCES += gui.cc tray.cc client.cc external.cc infobox.cc


RESOURCES = gui.qrc
