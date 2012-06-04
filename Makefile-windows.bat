@echo off
if "%BUILD_PLATFORM_ARCHITECTURE%." == "32." (
	set TARGET_PLATFORM_ARCHITECTURE_FLAG=/x86
) else (
	if "%BUILD_PLATFORM_ARCHITECTURE%." == "64." (
		set TARGET_PLATFORM_ARCHITECTURE_FLAG=/x64
	) else (
		echo BUILD_PLATFORM_ARCHITECTURE needs to be defined as 32 or 64
		goto :eof
	)
)

cd /d "C:\"
call "%MSSdk%\Bin\SetEnv.Cmd" %TARGET_PLATFORM_ARCHITECTURE_FLAG% /Release
cd /d %~dp0
set PATH=%PATH:Program Files (x86)=ProgramFiles86%
make %1 BUILD_PLATFORM=windows