#!/bin/bash

SOURCE=$1

if [ "$SOURCE" = "1" ]; then
	DEST=2
else
	DEST=1
fi

ifconfig tun1 10.11.10.${SOURCE} pointopoint 10.11.10.${DEST}
ifconfig tun1 10.11.10.${SOURCE} pointopoint 10.11.10.${DEST}
ifconfig tun1 10.11.10.${SOURCE} pointopoint 10.11.10.${DEST}

#until /sbin/ifconfig tun1; do echo Waiting for tun1; sleep 1; done


