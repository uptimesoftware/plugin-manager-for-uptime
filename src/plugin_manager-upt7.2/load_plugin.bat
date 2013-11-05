@echo off
cd ..
set UPTIME_DIR=%cd%
set PHPDIR=%UPTIME_DIR%\apache\php\
set LOADER_DIR=%UPTIME_DIR%\plugin_manager\
cd "%LOADER_DIR%"

"%PHPDIR%\php.exe" "%LOADER_DIR%\load_plugin.php" %1 %2 %3 %4 %5 %6 %7 %8 %9
