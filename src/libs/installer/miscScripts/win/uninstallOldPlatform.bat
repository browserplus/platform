@echo off

rem Uninstall old system-scoped BrowserPlus.  Quoting is a tad tricky, be careful
rem This is only needed during the transition from system-scope to 
rem user-scoped installs.
rem

set os=
ver > "%~dp0\osVer.tmp"
findstr /c:"Version 5" "%~dp0\osVer.tmp" >NUL 2>&1
if errorlevel 1 (
    set os=vista
) else (
    set os=xp
)
del /f /q "%~dp0\osVer.tmp"

echo Windows Registry Editor Version 5.00 > "%~dp0\nuke.reg"
echo. >> "%~dp0\nuke.reg"

echo [-HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Uninstall\Yahoo! BrowserPlus] >> "%~dp0\nuke.reg"

echo [-HKEY_CLASSES_ROOT\CLSID\{4852343E-3EFF-4971-A08A-4213E52C4E13}] >> "%~dp0\nuke.reg"

echo [-HKEY_CLASSES_ROOT\TypeLib\{399F26B4-E0C6-4345-8AD6-7AC1D86DAAA5}] >> "%~dp0\nuke.reg"
)

reg query HKEY_CLASSES_ROOT | find "Yahoo.BPCtl" > "%~dp0\tmp.txt"
for /f "usebackq delims==" %%k in ("%~dp0\tmp.txt") do (
    echo [-%%k] >> "%~dp0\nuke.reg"
)
del "%~dp0\tmp.txt" >NUL 2>&1

rem Remove "supress activex nattergram" entry and vista daemon elevation gunk
rem
reg query HKEY_USERS | find "\" > "%~dp0\tmp.txt"
for /f "usebackq delims==" %%u in ("%~dp0\tmp.txt") do (
    echo [-%%u\Software\Microsoft\Windows\CurrentVersion\Ext\Stats\{4852343E-3EFF-4971-A08A-4213E52C4E13}] >> "%~dp0\nuke.reg"
    )
)
del "%~dp0\tmp.txt" >NUL 2>&1

if %os% == vista (
    rem vista elevation for daemon
    rem
    reg query "HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\Low Rights\ElevationPolicy" > "%~dp0\tmp.txt"
    for /f "usebackq delims==" %%k in ("%~dp0\tmp.txt") do (
        reg query "%%k" /s | find "BrowserPlusCore.exe" > "%~dp0\tmp2.txt"
        for /f "usebackq delims==" %%j in ("%~dp0\tmp2.txt") do (
            echo [-%%k] >> "%~dp0\nuke.reg"
        )
        del "%~dp0\tmp2.txt" >NUL 2>&1
    )
)
del "%~dp0\tmp.txt" >NUL 2>&1

rem Runonce script from install/update
rem
reg query HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\RunOnce /s | find "RemoveOldBrowserPlus" > "%~dp0\tmp.txt"
for /f "usebackq delims==" %%k in ("%~dp0\tmp.txt") do (
    echo [HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\RunOnce] >> "%~dp0\nuke.reg"
    echo "RemoveOldBrowserPlus"=- >> "%~dp0\nuke.reg"
)

rem activex control appid
rem
echo [-HKEY_CLASSES_ROOT\AppId\YBPAddon.dll] >> "%~dp0\nuke.reg"

regedit /s "%~dp0\nuke.reg" >NUL 2>&1
del "%~dp0\nuke.reg" >NUL 2>&1

rem nuke IE plugin
rem
del /s /f /q "%ProgramFiles%\Internet Explorer\plugins\YBPAddon*" >NUL 2>&1

rem snort registry to find where firefox plugins live and remove them
rem
reg query HKLM\SOFTWARE\Mozilla /s | find "Plugins" > "%~dp0\tmp.txt"
for /f "usebackq tokens=2*" %%d in (`type "%~dp0\tmp.txt"`) do (
    del /s /f /q "%%e\npybrowserplus*" >NUL 2>&1
)
del "%~dp0\tmp.txt" >NUL 2>&1

rem ditto for safari.  of course, the format of the data from reg 
rem varies between xp and vista.  sigh.
rem
reg query HKLM\SOFTWARE\Clients\StartMenuInternet\Safari.exe\shell\open\command| find "REG_SZ" > "%~dp0\tmp.txt"
if %os% == vista (
    set tokenNum=2
) else (
    set tokenNum=3
)
for /f "usebackq tokens=%tokenNum%*" %%d in (`type "%~dp0\tmp.txt"`) do (
    del /s /f /q "%%~de%%~pePlugins\npybrowserplus*" >NUL 2>&1
)
del "%~dp0\tmp.txt" >NUL 2>&1

rem remove start menu items
rem
if %os% == vista (
    rmdir /s /q "%ALLUSERSPROFILE%\Microsoft\Windows\Start Menu\Programs\Yahoo! BrowserPlus" >NUL 2>&1 
) else (
    rmdir /s /q "%ALLUSERSPROFILE%\Start Menu\Programs\Yahoo! BrowserPlus" >NUL 2>&1 
)

rem Remove platform.
rem
rmdir /s /q "%ProgramFiles%\Yahoo! BrowserPlus" >NUL 2>&1

