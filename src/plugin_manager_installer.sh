#!/bin/sh

# -- program to install the Plugin Manager

##################################################################
# Variables
#
VERSION=1.1.1
PRODNAME="Plugin Manager"
UPTIMEDIR="/usr/local/uptime"
PLUGMANDIR="plugin_manager"
#
##################################################################

AWKBIN=`which awk`
PATH=$PATH:/usr/lib/sa:/usr/bin
OS=`uname`
export PATH


ID=`id | $AWKBIN 'BEGIN { FS="="; RS="(" } { print $2; exit }'`
if [ $ID -ne 0 ]; then
  echo "You must be root to install the up.time $PRODNAME"
  exit 1
fi

echo
echo "$PRODNAME $VERSION Installation"
echo "-------------------------------------------"
echo "The $PRODNAME will be installed under '$UPTIMEDIR'."
echo "If up.time is installed in a different directory you will be"
echo "able to specify it below."
echo
echo "The installation will not make any changes to up.time, but it will restart the up.time Web Server (uptime_httpd)."
echo "It will install the following:"
echo "- $UPTIMEDIR/$PLUGMANDIR"
echo "- $UPTIMEDIR/GUI/$PLUGMANDIR"
echo
echo "Press enter to continue"
#echo "To continue please press 'Y', to quit press 'N': "
# read screen input
read a

# Installation will continue
echo
echo "Is the up.time monitoring station installed in the default location"
echo "(/usr/local/uptime)? (default)Y or N : "
read a
a=`echo $a | tr '[a-z]' '[A-Z]'`

if [ "$a" != "Y" -a "$a" != "" ] ; then
	echo "Please specify the path to where the up.time monitoring station is installed:"
	read UPTIMEDIR
	if  [  ! -d "$UPTIMEDIR" ] ; then
		echo "Error: Could not find the directory: '$UPTIMEDIR'."
		echo "Exiting $PRODNAME installer."
		exit 2
	fi
fi


# INSTALL
echo
echo "$PRODNAME will be installed in the following directory:"
echo "'$UPTIMEDIR/$PLUGMANDIR'."
echo "Are you ready to install? (default)Y or N : "
read a
a=`echo $a | tr '[a-z]' '[A-Z]'`
if [ "$a" != "Y" -a "$a" != "" ]; then
	echo "Installation has been cancelled by user."
	echo "You can install it later."
	exit 1
fi



# Backup files if upgrading
TEMPDBBACKUP="./db"

if  [ -d "$UPTIMEDIR/$PLUGMANDIR/db/" ] ; then
	if [ ! -d "$TEMPDBBACKUP" ] ; then
		echo "Creating backup of plugins."
		mkdir $TEMPDBBACKUP
		cp -rf $UPTIMEDIR/$PLUGMANDIR/db/* $TEMPDBBACKUP/.
	else
		echo "Error: '$TEMPDBBACKUP' directory already exists. Delete/move it and try again."
		echo "Exiting $PRODNAME installer."
		exit 3
	fi
fi


echo "Copying files."
rm -rf $UPTIMEDIR/$PLUGMANDIR
rm -rf $UPTIMEDIR/GUI/$PLUGMANDIR

cp -rf ./$PLUGMANDIR/ $UPTIMEDIR/.
cp -rf $UPTIMEDIR/$PLUGMANDIR/GUI/$PLUGMANDIR/ $UPTIMEDIR/GUI/.

echo "Setting permissions."
chown -R uptime $UPTIMEDIR/$PLUGMANDIR
chown -R uptime $UPTIMEDIR/GUI/$PLUGMANDIR

chmod 755 $UPTIMEDIR/$PLUGMANDIR/*.php $UPTIMEDIR/$PLUGMANDIR/*.sh
chmod 755 $UPTIMEDIR/GUI/$PLUGMANDIR/*.php

chown root $UPTIMEDIR/$PLUGMANDIR/bin/*
chmod 4755 $UPTIMEDIR/$PLUGMANDIR/bin/*

if [ -d "$TEMPDBBACKUP" ] ; then
	echo "Restoring backup of plugins."
	cp -rf $TEMPDBBACKUP $UPTIMEDIR/$PLUGMANDIR/.
	chown -R uptime $UPTIMEDIR/$PLUGMANDIR/db
	rm -rf $TEMPDBBACKUP
fi

echo "Fixing PHP limitations."
cd $UPTIMEDIR/$PLUGMANDIR; ./fix_php_limitations.sh
echo "Restarting the up.time Web Server (uptime_httpd)."
/etc/init.d/uptime_httpd stop
/etc/init.d/uptime_httpd start


HOSTNAME=`hostname`
if [ -f "/usr/local/uptime/apache/conf/httpd.conf" ]; then
	UPTIMEPORT=`cat "/usr/local/uptime/apache/conf/httpd.conf" | grep "^Listen" | cut -c 8-`
elif [ -f "/opt/uptime/apache/conf/httpd.conf" ]; then
	UPTIMEPORT=`cat "/opt/uptime/apache/conf/httpd.conf" | grep "^Listen" | cut -c 8-`
elif [ -f "$UPTIMEDIR/apache/conf/httpd.conf" ]; then
	UPTIMEPORT=`cat "$UPTIMEDIR/apache/conf/httpd.conf" | grep "^Listen" | cut -c 8-`
else
	UPTIMEPORT=9999
fi

echo
echo "------------------------------------------------------------------------------"
echo
echo "NOTE:"
echo "To complete installation you must make the following changes to the"
echo "up.time configuration:"
echo " - Login to the up.time monitoring station interface"
echo " - Click on Config -> up.time Configuration"
echo " - Enter the following and click the Update button:"
echo ""
echo "myportal.custom.tab1.enabled=true"
echo "myportal.custom.tab1.name=up.time Plugin Manager"
echo "myportal.custom.tab1.URL=/plugin_manager/"
echo
echo "------------------------------------------------------------------------------"
echo
echo "Done installing $PRODNAME."
exit 0

# -- The End.
