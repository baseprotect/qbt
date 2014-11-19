strace_win:{
  contains(QMAKE_HOST.arch, x86):{
    # i686 arch requires frame pointer preservation
    QMAKE_CXXFLAGS_RELEASE += -Oy-
    QMAKE_CXXFLAGS_DEBUG += -Oy-
  }
  release:{
    QMAKE_CXXFLAGS_RELEASE += -Zi
    QMAKE_LFLAGS += "/DEBUG"
  }
  LIBS += dbghelp.lib
}

QMAKE_LFLAGS += "/OPT:REF /OPT:ICF"

RC_FILE = qbittorrent.rc

# Enable Wide characters
DEFINES += TORRENT_USE_WPATH

#Adapt the lib names/versions accordingly
CONFIG(debug, debug|release) {
  LIBS += libtorrent.lib \
          libboost_system-vc100-mt-gd-1_55.lib \
          libboost_filesystem-vc100-mt-gd-1_55.lib \
          libboost_thread-vc100-mt-gd-1_55.lib \
          libboost_date_time-vc100-mt-gd-1_55.lib
} else {
  LIBS += libtorrent.lib \
          libboost_system-vc100-mt-1_55.lib \
          libboost_filesystem-vc100-mt-1_55.lib \
          libboost_thread-vc100-mt-1_55.lib \
          libboost_date_time-vc100-mt-1_55.lib
}

LIBS += libsoci_core_3_2.lib \
        libsoci_sqlite3_3_2.lib \
        sqlite3.lib

LIBS += advapi32.lib shell32.lib crypt32.lib User32.lib
#LIBS += libeay32.lib ssleay32.lib
LIBS += PowrProf.lib
#LIBS += zlib.lib

#ENABLE BASEPROTECT MOD
DEFINES += BASEPROTECT
