#!/bin/bash

#sudo -i
cd /home/dan/target_gnuradio/
source setup_env.sh
cd src/gr-ccsds/examples

SOURCE=$1
if [ $# -eq 2 ]; then
	RATE=$2
else
	RATE=500000
fi

echo "Data rate is set to $RATE bps"

if [ "$SOURCE" = "1" ]; then
	DEST=2
	python concatenated_qpsk_modem_txrx.py --mtu 1115 --rx-freq=400M --tx-freq=415M --gain 35 --tx-user-data-rate=$RATE --rx-user-data-rate=$RATE #&
else
	DEST=1
	python concatenated_qpsk_modem_txrx.py --mtu 1115 --tx-freq=400M --rx-freq=415M --gain 35 --rx-user-data-rate=$RATE --tx-user-data-rate=$RATE #&
fi

#sleep 10

#ifconfig tun1 10.11.10.${SOURCE} pointopoint 10.11.10.${DEST}
#ifconfig tun1 10.11.10.${SOURCE} pointopoint 10.11.10.${DEST}
#ifconfig tun1 10.11.10.${SOURCE} pointopoint 10.11.10.${DEST}

#until /sbin/ifconfig tun1; do echo Waiting for tun1; sleep 1; done


