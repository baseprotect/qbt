PROJECT_NAME = bpqbittorrent
PROJECT_VERSION = 1.0.0

os2 {
    DEFINES += VERSION=\'\"v$${PROJECT_VERSION}\"\'
} else {
    DEFINES += VERSION=\\\"v$${PROJECT_VERSION}\\\"
}

DEFINES += VERSION_MAJOR=3
DEFINES += VERSION_MINOR=1
DEFINES += VERSION_BUGFIX=9