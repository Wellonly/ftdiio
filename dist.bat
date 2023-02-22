
@REM - copy structure to destination build folder
@REM - copy, generate data files into folder with deploy
@REM - Generate installer with binarycreator

@REM USAGE: define deploy: dist.bat install_dir install_name source_fname
@REM USAGE: define path to binarycreator.exe

echo OFF
set INSDIR=%1
set INSNAME=%2
set RUNFNAME=%3

if %INSDIR% == "" goto bad_param
if %INSNAME% == "" goto bad_param
if %RUNFNAME% == "" goto bad_param

set DESTFOLD=%INSDIR%\%INSNAME%

set LIBDLL=E:\libs\ftdi\i386\ftd2xx.dll
set DRVX64=E:\ins\drivers\FTDI\CDM21228_Setup.exe
set DRVX86=E:\ins\drivers\FTDI\CDM21228_Setup.exe
set DESTDATA=%DESTFOLD%\packages\org.install.exe\data
set DESTDRVX86=%DESTFOLD%\packages\org.install.drvx86\data\DRV
set DESTDRVX64=%DESTFOLD%\packages\org.install.drvx64\data\DRV

if not exist %RUNFNAME% goto srcferror
if not exist %LIBDLL% goto drvferror
if not exist %DRVX64% goto drvferror
if not exist %DRVX86% goto drvferror

mkdir %DESTFOLD% 2>&1 || goto error

xcopy dist %DESTFOLD% /E 2>&1 || goto error

xcopy %RUNFNAME% %DESTDATA% 2>&1 || goto error

xcopy %LIBDLL% %DESTDATA% 2>&1 || goto error
xcopy %DRVX64% %DESTDRVX64% 2>&1 || goto error
xcopy %DRVX86% %DESTDRVX86% 2>&1 || goto error

windeployqt.exe --qmldir %DESTDATA%\QML %DESTDATA% 2>&1 || goto error

binarycreator.exe --offline-only -c %DESTFOLD%\config\config.xml -r %DESTFOLD%\resources\additional.qrc -p %DESTFOLD%\packages %DESTFOLD%\%INSNAME% 2>&1 || goto error

echo %INSDIR% %INSNAME% %RUNFNAME%
echo distribution built successfully!!!
exit /B 0

:bad_param
echo bad parameters! See the USAGE notes!
exit /B 1

:error
echo distribution build stopped on error!
exit /B 2

:srcferror
echo distribution build stopped! Source file.exe not found!
exit /B 3

:drvferror
echo distribution build stopped! Driver file(s) not found!
exit /B 4
