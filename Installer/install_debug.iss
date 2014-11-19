
#define JoinerGetSuffix() GetDateTimeString('yyyy_mm_dd','','');

[Setup]
AppName=Baseprotect QBT
AppVersion=1.0
DefaultDirName="{pf}\BaseprotectQBT"
OutputDir=bin
OutputBaseFilename=Install_Debug_BPQBT_{#JoinerGetSuffix()}

[InstallDelete]
Type: filesandordirs; Name: "{localappdata}\qBittorrent"
Type: filesandordirs; Name: "{userappdata}\qBittorrent"

[Files]
Source: "..\qbittorrent-build-desktop-Qt_4_8_5__4_8_5__Debug\src\debug\qbittorrent.exe"; DestDir: "{pf}/BaseprotectQBT"; Flags: ignoreversion
Source: "..\qbittorrent-3.1.9\src\baseprotect\scripts\*.*"; DestDir: "{pf}/BaseprotectQBT/scripts"; Flags: ignoreversion
Source: "translations\*.*"; DestDir: "{pf}/BaseprotectQBT/translations"; Flags: ignoreversion
Source: "qt.conf"; DestDir: "{pf}/BaseprotectQBT"; Flags: ignoreversion
Source: "debug/*.dll";  DestDir: "{pf}/BaseprotectQBT"; Flags: ignoreversion

[Run]
Filename: "{pf}\BaseprotectQBT\qbittorrent.exe"; Flags: nowait runminimized postinstall