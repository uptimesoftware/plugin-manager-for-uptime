#!/bin/bash

rm -rf /usr/local/uptime/plugin_manager
rm -rf /usr/local/uptime/GUI/plugin_manager
rm -f plugin_manager_installer.sh
rm -rf plugin_manager

cp -R /mount/home/jpereira/FILES/install_plugin_manager/plugin_manager .
cp -R /mount/home/jpereira/FILES/install_plugin_manager/plugin_manager_installer.sh .

chown -R uptime /usr/local/uptime/plugin_manager
chown -R uptime /usr/local/uptime/GUI/plugin_manager

chmod 755 /usr/local/uptime/plugin_manager/*.php /usr/local/uptime/plugin_manager/*.sh
chmod 755 /usr/local/uptime/GUI/plugin_manager/*.php
