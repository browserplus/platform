@echo off
rem ***************************************************************************
rem *
rem *  A simple bat file that will use ServiceInstaller.exe to install the
rem *  sample service for you (once you've built it).
rem *
rem ***************************************************************************

cls
IF NOT EXIST InstallThisService.bat goto RunnningFromWrongDir
IF NOT EXIST ..\..\bin\ServiceInstaller.exe goto MissingInstaller
IF NOT "%1" == "" goto HasArgument
IF NOT EXIST SampleService goto MissingDir
IF NOT EXIST SampleService\manifest.json goto MissingDll
IF NOT EXIST SampleService\SampleService.dll goto MissingDll

rem ***************************************************************************
rem * This is what installs the sevice.
rem * All that's needed is to call:
rem *     ..\..\bin\ServiceInstaller [opts] <service dir>
rem *
rem * WARNING: The -f option here forces overwriting an existing service
rem *          of the same name.
rem ***************************************************************************
echo.
echo Using ServiceInstaller.exe to install "SampleService":
echo.
..\..\bin\ServiceInstaller -v -f SampleService
goto TheEnd

:HasArgument
echo.
echo    ERROR: This bat file does NOT take arguments.
echo.
echo.
echo    If you want to specify arguments then
echo    you should use ServiceInstaller.exe directly.
echo    Here's the usage help from ServiceInstaller.exe:
echo.
..\..\bin\ServiceInstaller -h
goto TheEnd

:RunnningFromWrongDir
echo.
echo    ERROR: This bat file MUST be run from the directory in which it resides
echo           (because it uses relative pathing).
echo.
goto TheEnd

:MissingInstaller
echo.
echo    ERROR: The BrowserPlus SDK's ServiceInstaller.exe is not located
echo           where expected. It should be at ..\..\bin\ServiceInstaller.exe
echo.
goto TheEnd

:MissingDir
echo.
echo    ERROR: The directory "SampleService" does not exist.
echo            You must build the service before it can be installed.
echo.
goto TheEnd

:MissingManifest
echo.
echo    ERROR: The file "SampleService\manifest.json" does not exist.
echo           You must build the service before it can be installed.
echo.
goto TheEnd

:MissingDll
echo.
echo ERROR: The file "SampleService\SampleService.dll" does not exist.
echo        You must build the service before it can be installed.
echo.
goto TheEnd

:TheEnd
rem ***************************************************************************
rem * This annoying pause is here in case someone runs this from explorer
rem * rather than the command-line. In which case it stops the dos box from
rem * disappearing before the user can read what happened.
rem ***************************************************************************
echo.
pause
