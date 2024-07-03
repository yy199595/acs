@echo off

set outputDir=../bin/www/dist/
call npm install
echo Installing dependencies...

echo Building Vue project...
call npm run build
rmdir /s/q "%outputDir%"
echo Copying output directory...
xcopy /e /i /y "./dist" "%outputDir%"

pause
