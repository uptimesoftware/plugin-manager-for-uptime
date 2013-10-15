@echo off

copy /Y /B /V plugin_manager_installer.sh M:\jpereira\FILES\install_plugin_manager
mkdir M:\jpereira\FILES\install_plugin_manager\plugin_manager\
xcopy /E /V /R /Y plugin_manager-upt7.1 M:\jpereira\FILES\install_plugin_manager\plugin_manager\

pause