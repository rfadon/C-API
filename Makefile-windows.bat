cd /d "C:\"
call "%MSSdk%\Bin\SetEnv.Cmd" /x64 /Release
cd /d %~dp0
set PATH=%PATH:Program Files (x86)=ProgramFiles86%
make %1 BUILD_PLATFORM=windows