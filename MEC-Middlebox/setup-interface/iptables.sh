#!/bin/bash
sudo iptables -t nat -A PREROUTING -p udp -d 172.17.1.4 --dport 2152 -j DNAT --to-destination 172.17.1.2:7000
#sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -d 38.68.232.117 -j DROP


