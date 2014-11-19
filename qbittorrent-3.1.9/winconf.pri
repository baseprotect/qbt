# Adapt these paths on Windows

#Point this to the boost include folder
INCLUDEPATH += $$quote(C:\WORK\boost_1_55_0)
#Point this to the libtorrent include folder
INCLUDEPATH += $$quote(C:\WORK\libtorrent-rasterbar-0.16.13\libtorrent-rasterbar-0.16.13\include)
#Point this to the zlib include folder
INCLUDEPATH += $$quote(C:\WORK\zlib-vc\zlib\include)
#Point this to the openssl include folder
#INCLUDEPATH += $$quote(C:\OpenSSL\include)

#Points to sqlite include folder
INCLUDEPATH += $$quote(C:\WORK\sqlite-dev)
#Points to soci core include folder
INCLUDEPATH += $$quote(C:\WORK\soci-3.2.2\soci-3.2.2\core)
#Points to soci sqlite backend include folder
INCLUDEPATH += $$quote(C:\WORK\soci-3.2.2\soci-3.2.2\backends\sqlite3)

LIBS += $$quote(-LC:\WORK\boost_1_55_0\stage\lib)
#Point this to the libtorrent lib folder
CONFIG(debug, debug|release) {
LIBS += $$quote(-LC:\WORK\libtorrent-rasterbar-0.16.13\libtorrent-rasterbar-0.16.13\bin\msvc-10.0\debug\boost-source\link-static\threading-multi\)
}else{
LIBS += $$quote(-Lc:\WORK\libtorrent-rasterbar-0.16.13\libtorrent-rasterbar-0.16.13\bin\msvc-10.0\release\boost-source\link-static\threading-multi\)
}
#Point this to the zlib lib folder
LIBS += $$quote(-LC:\WORK\zlib-vc\zlib\lib)
#OpenSSL
LIBS += $$quote(-LC:\OpenSSL)
#SOCI
CONFIG(debug, debug|release) {
LIBS += $$quote(-LC:/WORK/soci-3.2.2/soci-3.2.2/lib/Debug/)
}else{
LIBS += $$quote(-LC:/WORK/soci-3.2.2/soci-3.2.2/lib/Release/)
}
#SQLITE
LIBS += $$quote(-LC:\WORK\sqlite-dev)


# LIBTORRENT DEFINES
DEFINES += BOOST_ALL_NO_LIB
DEFINES += BOOST_ASIO_HASH_MAP_BUCKETS=1021
DEFINES += BOOST_ASIO_SEPARATE_COMPILATION
DEFINES += BOOST_EXCEPTION_DISABLE
DEFINES += BOOST_SYSTEM_STATIC_LINK=1
# DEFINES += TORRENT_USE_OPENSSL
DEFINES += UNICODE
DEFINES += _UNICODE
DEFINES += WIN32
DEFINES += _WIN32
DEFINES += WIN32_LEAN_AND_MEAN
DEFINES += _WIN32_WINNT=0x0500
DEFINES += _WIN32_IE=0x0500
DEFINES += _CRT_SECURE_NO_DEPRECATE
DEFINES += _SCL_SECURE_NO_DEPRECATE
DEFINES += __USE_W32_SOCKETS
DEFINES += _FILE_OFFSET_BITS=64
DEFINES += WITH_SHIPPED_GEOIP_H

CONFIG(debug, debug|release) {
  DEFINES += TORRENT_DEBUG
} else {
  DEFINES += NDEBUG
}

#Enable backtrace support
win32-msvc* {
  CONFIG += strace_win
}

strace_win:{
  DEFINES += STACKTRACE_WIN
  FORMS += stacktrace_win_dlg.ui
  HEADERS += stacktrace_win.h \
             stacktrace_win_dlg.h
}

win32-g++ {
  include(winconf-mingw.pri)
}
else {
  include(winconf-msvc.pri)
}

DEFINES += WITH_GEOIP_EMBEDDED
message("On Windows, GeoIP database must be embedded.")

DEFINES += BOOST_TT_HAS_OPERATOR_HPP_INCLUDED
