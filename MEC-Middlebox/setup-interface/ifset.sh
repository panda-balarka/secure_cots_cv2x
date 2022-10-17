#!/bin/bash
# TODO: Change EPC_IF and EPC_MAC for your NIC
EPC_IF=enp4s0
EPC_MAC=14:b3:1f:03:80:0d


# TODO: Change eNB_IF and eNB_MAC for your NIC
eNB_IF=enxb4b024158db4
eNB_MAC=00:05:1b:c4:b8:41

sudo ifconfig $EPC_IF up
sudo ifconfig $eNB_IF up

sudo ifconfig $EPC_IF 172.17.1.3/32
sudo ifconfig $eNB_IF 172.17.1.2/32

sudo arp -i $EPC_IF -s 172.17.1.1 $EPC_MAC pub
sudo arp -i $eNB_IF -s 172.17.1.4 $eNB_MAC pub

sudo route add -host 172.17.1.1 $eNB_IF
sudo route add -host 172.17.1.4 $EPC_IF

#sudo route add -net 192.172.0.0 gw 172.17.1.1 netmask 255.255.255.0 dev $eNB_IF

sudo sysctl -w net.ipv4.ip_forward=1

