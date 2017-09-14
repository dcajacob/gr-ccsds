# Demo Instructions

On one computer, run ./grcon_demo.sh 1, then ./tunup.sh 1

On the other computer, run ./grcon_demo.sh 2, then ./tunup.sh 2

Now you can network between the systems, over the RF link.  For example:

From system 1:

ping -A 10.11.10.2

ssh -vvv user@10.11.10.2

scp -rv file user@10.11.10.2

Note that file transfer may stall.  This may be due to the TCP/IP stack congestion window being 
reduced when it mistakenly identifies a packet loss or long RTT time as congestion on a normal 
network.  We should be able to fix this with a PEP.  Another possibility is that we're sending 
a packet that exceeds the MTU of the TUN, but I don't think this should be possible.

I usually use the following custom settings for OTA SSH (just add them to your ssh config file 
for the link you want to use:

Host usrp
Hostname 10.11.10.2
User hawk
Port 22
StrictHostKeyChecking no
#PreferredAuthentications password
#PubkeyAuthentication no
#PasswordAuthentication yes
ChallengeResponseAuthentication no
CheckHostIP no
ConnectTimeout 300
ConnectionAttempts 10
ServerAliveInterval 15
ServerAliveCountMax 4
TCPKeepAlive no
Cipher blowfish-cbc

More fun:

rsync -rzhPit --bwlimit=45k --stats --sockopts tcp_frto=1,tcp_frto_response=3,tcp_slow_start_after_idle=0,tcp_keepalive_probes=100,tcp_keepalive_intvl=3,tcp_low_latency=1,TCP_CONGESTION=vegas,TCP_MAXSEG=1113 Downloads/AIS.SampleData.zip  usrp:~/Downloads/

It's often a good idea to keep some backround pinging going.  Try not to overwhelm the link though, unless you're using some sort of QOS

ping -i 1 10.11.10.2

# Network Test (no hardware)

./concatenated_qpsk_modem_txrx_net.py --mtu 1115 --source-ip 192.168.1.231 --dest-ip 192.168.1.223 --source-port 52001 --dest-port 52002 --rx-user-data-rate 1000000 --tx-user-data-rate 1000000

./concatenated_qpsk_modem_txrx_net.py --mtu 1115 --source-ip 192.168.1.223 --dest-ip 192.168.1.231 --dest-port 52001 --source-port 52002 --rx-user-data-rate 1000000 --tx-user-data-rate 1000000
