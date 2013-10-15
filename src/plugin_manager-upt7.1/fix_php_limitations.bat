@echo off
set CUR_DIR="%cd%"

REM Use the first argument as the uptime directory path
SET UPTIME_DIR=%1%
REM Remove any double quotes that the installer sends
SET UPTIME_DIR=%UPTIME_DIR:"=%

SET PHP_DIR=%UPTIME_DIR%\apache\php
SET PHP_CMD=%UPTIME_DIR%\plugin_manager\fix_php_limitations.php

SET FULL_CMD="%PHP_DIR%\php.exe" "%PHP_CMD%" "%PHP_DIR%\php.ini"

%FULL_CMD%

echo "Restarting up.time Web Server Service"
net stop "up.time Web Server"
net start "up.time Web Server"
