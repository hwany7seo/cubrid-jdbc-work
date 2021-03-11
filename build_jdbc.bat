@echo off
set CUR_PATH=%cd%
set ANT_PATH=%ANT_HOME%
set ANT_FILE=%ANT_HOME%\bin\ant
set JAVA_PATH=%JAVA_HOME%
set JAVA_FILE=%JAVA_HOME%\bin\java.exe
set VERSION_FILE=VERSION

if "%JAVA_PATH%" == "" (
	echo "JAVA path(%JAVA_PATH%) is not exist."
	goto :eof 
) else if EXIST %JAVA_FILE% (
	goto :BUILD
) else (
	echo "JAVA file(%JAVA_FILE%) is not exist."
	goto :eof 
)

if "%ANT_PATH%" == "" (
	echo "ANT Path is not exist."
) else if exist %ANT_FILE% (
	goto :BUILD
) else (
	echo "ANT excute file is not exist. %ANT_FILE%"
)

:BUILD
::echo "ANT excute %ANT_FILE%"
for /f "delims=" %%i in (%CUR_PATH%\%VERSION_FILE%) do set VERSION=%%i
echo "VERSION = %VERSION%"
mkdir output
copy VERSION output\CUBRID-JDBC-%VERSION%
if "%1" == "clean" (
	%ANT_FILE% clean -buildfile ./build.xml
) else (
	%ANT_FILE% dist-cubrid -buildfile ./build.xml -Dbasedir=. -Dversion=%VERSION% -Dsrc=./src
)
 
