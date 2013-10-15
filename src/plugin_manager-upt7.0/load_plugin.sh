#!/bin/sh
cd ..
UPTIME_DIR=`pwd`

if [ -e "/usr/local/uptime/apache/bin/php" ]; then
  PHPDIR="/usr/local/uptime/apache/bin/"
elif [ -e "/opt/uptime/apache/bin/php" ]; then
  PHPDIR="/opt/uptime/apache/bin/"
else
  PHPDIR="/usr/local/uptime/apache/bin"
  echo "ERROR (load_plugin.sh): Could not confirm apache directory!"
fi

LOADER_DIR="$UPTIME_DIR/plugin_manager/"
cd "$LOADER_DIR"

"$PHPDIR/php" "$LOADER_DIR/load_plugin.php" $1 $2 $3 $4 $5 $6 $7 $8 $9
