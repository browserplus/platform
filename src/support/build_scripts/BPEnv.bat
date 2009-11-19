@ECHO OFF
REM Setup console environment for BrowserPlus development.

REM Setup for BuildTemplates usage
CALL "%~dp0\..\External\ycpbin\ycpsdk_M9_BP-Windows-5.1-vs80\share\BuildTemplates\Scripts\YBTEnv.bat"

REM Setup path to cmake.exe, etc.
SET PATH=%PATH%;%~dp0\..\External\ycpbin\ycpsdk_M9_BP-Windows-5.1-vs80\bin

REM Setup path to ruby.exe, etc.
REM In order to have ycp\External\Windows\bin you have to run
REM buildall.bat in the ycp directory.
SET PATH=%PATH%;%~dp0\..\External\ycp\External\Windows\bin

TITLE Yahoo! BrowserPlus Prompt