@echo off

set CUR_PATH=%cd%
set ANT_FILE=%ANT_HOME%\bin\ant
set JAVA_FILE=%JAVA_HOME%\bin\java.exe
set VERSION_FILE=VERSION
set GIT_PATH=C:\Program Files\Git\bin\git.exe
set EXTRA_START_DATE=2021-03-01
echo Checking for requirements...

if "%JAVA_HOME%" == "" (
	echo "JAVA_HOME path is not exist."
	goto :eof 
) else if EXIST %JAVA_FILE% (
	goto :BUILD
) else (
	echo "JAVA file(%JAVA_FILE%) is not exist."
	goto :eof 
)

if "%ANT_HOME%" == "" (
	echo "ANT_HOME is not exist."
) else if exist %ANT_FILE% (
	goto :BUILD
) else (
	echo "ANT excute file is not exist. %ANT_FILE%"
)

:BUILD
::echo "ANT excute %ANT_FILE%"
for /f "delims=" %%i in (%CUR_PATH%\%VERSION_FILE%) do set VERSION=%%i

call :FINDEXEC git.exe GIT_PATH "%GIT_PATH%"

if EXIST "%CUR_PATH%\\.git" (
  for /f "delims=" %%i in ('"%GIT_PATH%" rev-list --after %EXTRA_START_DATE% --count HEAD') do set EXTRA_VERSION=%%i
) else (
  set EXTRA_VERSION=0000
)

echo "VERSION = %VERSION%.%EXTRA_VERSION%"
if "%1" == "clean" (
  %ANT_FILE% clean -buildfile ./build.xml
) else (
  if NOT EXIST "%CUR_PATH%\\output" (
    mkdir output
  )
  copy VERSION output\CUBRID-JDBC-%VERSION%.%EXTRA_VERSION%
  %ANT_FILE% dist-cubrid -buildfile ./build.xml -Dbasedir=. -Dversion=%VERSION%.%EXTRA_VERSION% -Dsrc=./src
)
 
:FINDEXEC
if EXIST %3 set %2=%~3
if NOT EXIST %3 for %%X in (%1) do set FOUNDINPATH=%%~$PATH:X
if defined FOUNDINPATH set %2=%FOUNDINPATH:"=%
if NOT defined FOUNDINPATH if NOT EXIST %3 echo Executable [%1] is not found & GOTO :EOF
call echo Executable [%1] is found at [%%%2%%]
GOTO :EOF