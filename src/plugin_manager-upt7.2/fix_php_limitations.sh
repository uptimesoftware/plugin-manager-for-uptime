#!/bin/sh
CUR_DIR=`pwd`
cd ..
UPTIME_DIR=`pwd`

if [ -e "/usr/local/uptime/apache/bin/php" ]; then
  PHP_DIR="/usr/local/uptime/apache/bin/"
  PHP_INI_FILE="/usr/local/uptime/apache/conf/php.ini"
elif [ -e "/opt/uptime/apache/bin/php" ]; then
  PHP_DIR="/opt/uptime/apache/bin/"
  PHP_INI_FILE="/opt/uptime/apache/conf/php.ini"
else
  PHP_DIR="/usr/local/uptime/apache/bin"
  PHP_INI_FILE="/usr/local/uptime/apache/conf/php.ini"
  echo "WARNING (fix_php_limitations.sh): Could not confirm APACHE/PHP directory!"
fi

cd "$CUR_DIR"

"$PHP_DIR/php" "$CUR_DIR/fix_php_limitations.php" "$PHP_INI_FILE"
