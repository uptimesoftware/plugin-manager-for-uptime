#!/bin/sh

# -- program to install the Plugin Manager

##################################################################
# Variables
#
VERSION=1.2.0
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
cp -f $UPTIMEDIR/$PLUGMANDIR/mibs/* $UPTIMEDIR/mibs/

echo "Setting permissions."
chown -R uptime $UPTIMEDIR/$PLUGMANDIR
chown -R uptime $UPTIMEDIR/GUI/$PLUGMANDIR
chown -R uptime:uptime $UPTIMEDIR/mibs

chmod 755 $UPTIMEDIR/$PLUGMANDIR/*.php $UPTIMEDIR/$PLUGMANDIR/*.sh
chmod 755 $UPTIMEDIR/GUI/$PLUGMANDIR/*.php
chmod 755 $UPTIMEDIR/mibs/*

chown root $UPTIMEDIR/$PLUGMANDIR/bin/*
chmod 4755 $UPTIMEDIR/$PLUGMANDIR/bin/*

if [ -f /etc/redhat-release ]
then
    NUM_WORDS=`cat /etc/redhat-release |wc -w`
    VERSION_POS=`expr ${NUM_WORDS} - 1`
    REDHAT_VERSION=`cat /etc/redhat-release |cut -d\  -f ${VERSION_POS} | cut -d. -f 1`
    if [ "${REDHAT_VERSION}" == "6" ]
    then
        cp $UPTIMEDIR/$PLUGMANDIR/bin/restart_core-linux-6x.bin $UPTIMEDIR/$PLUGMANDIR/bin/restart_core-linux.bin
    else 
        cp $UPTIMEDIR/$PLUGMANDIR/bin/restart_core-linux-5x.bin $UPTIMEDIR/$PLUGMANDIR/bin/restart_core-linux.bin
    fi
else
    cp $UPTIMEDIR/$PLUGMANDIR/bin/restart_core-linux-5x.bin $UPTIMEDIR/$PLUGMANDIR/bin/restart_core-linux.bin
fi 

if [ -d "$TEMPDBBACKUP" ] ; then
	echo "Restoring backup of plugins."
	cp -rf $TEMPDBBACKUP $UPTIMEDIR/$PLUGMANDIR/.
	chown -R uptime $UPTIMEDIR/$PLUGMANDIR/db
	rm -rf $TEMPDBBACKUP
fi


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
