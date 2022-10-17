# MEC Middlebox prototype

###### tags: `MEC`

## Premilinary:
* Working OAI EPC and RAN

* Connect your MEC to RAN and to EPC with two different cables(NIC1, NIC2 respectively)

* Follow SetupMEC.md to install GTP kernel module which supports our platform.

## Setup

- The example configuration files are in `setup-interface`

* Since the network is divided into two segments, RAN and EPC have can't receive ARP request from each other. Set up MEC as a proxy ARP server for each end. 
    1. Broadcast MEC MAC address to EPC on NIC2, on behalf of RAN
    ```
    sudo arp -i <NIC2> -s <RAN IP> <NIC2 MAC> pub
    ```
    * parameters: 
        * NIC2: interface name of NIC2
        * NIC2 MAC: MAC address of NIC2
    2. Broadcast MEC MAC address to RAN on NIC1, on behalf of EPC
    ```
    sudo arp -i <NIC1> -s <EPC IP> <NIC1 MAC> pub
    ```
    * parameters:
        * NIC1: interface name of NIC1
        * NIC1 MAC: MAC address of NIC1
* Forward traffic from RAN/EPC to EPC/RAN
    1. Set routing rules to forward traffic destined for EPC on NIC2
    ```
    sudo route add -host <EPC IP>  <NIC2>
    ```
    2. Set routing rules to forward traffic destined for RAN on NIC1
    ```
    sudo route add -host <RAN IP>  <NIC1>
    ```
* Set routing rules for traffic to UE
    ```
    sudo route add -net <UE SUBNET> gw <RAN IP> netmask 255.255.255.0 dev <NIC1>
    ```
    * parameter:
        * UE SUBNET: CIDR of the UE SUBNET (e.g. 192.172.0.0)

* Set iptables rule to forward traffic to your MEC machine 
    ```
    sudo iptables -t nat -A PREROUTING -p udp -d <EPC IP> --dport 2152 -j DNAT --to-destination <MEC_IP>:<MEC_PORT>
    ```
    * parameters:
        * MEC IP: your MEC IP
        * MEC PORT: Port binded for the module dispatcher below
7. Enable ip forwarding in your OS
```
sudo sysctl -w net.ipv4.ip_forward=1
```



## Purposes of each module

* dispatcher: Redirects network traffic 
* virtual-Interface: Encapsulate/Decapsulate GTP tunnel headers


## Usage:
1. Set up your config files: LOCAL_SERVICE (see more description below)
2. run two commands below to start GTP packet handling
```
$ touch <LOCAL SERVICE> (e.g. touch LocalService)
$ sudo ./dis/dist/dispatcher/dispatcher <MEC_IP> <MEC_PORT> <CORE_IP> <LocalService>
```

```
$ sudo ./virtualInterface <UE_Size> <UE_Netmask> <UE_Subnet> <MTU> <CORE_IP> <eNB_IP> <MEC_IP>
```

## Required parameters:
* MEC_IP / MEC_PORT: the IP/PORT of your MEC machine where dispatcher runs
* CORE_IP: IP of your EPC S-GW
* LOCAL_SERVICE: a file containing IP binded to your local services, each line consists with one IP and a '\n' character
    * e.g. if your apache server is binded on 10.0.0.1 and a new line containing 10.0.0.1


* eNB_IP: IP of your RAN
* UE_Size: Maximum size of your UEs
* UE_Netmask: Netmask of your UEs' IP (e.g. 24)
* UE_Subnet: UE subent range/24 (e.g. 192.172.0.0)
* MTU: MTU of virtual interface (e.g. 1500)

## Enable DNS server on ubuntu

- Install dnsmasq
```
$ sudo apt install dnsmasq
```

- Start/Restart/Stop dnsmasq
```
$ sudo service dnsmasq start/restart/stop
```

- Change `/etc/dnsmasq.conf` to configure your own DNS server

```
$ sudo vim /etc/dnsmasq.conf
```

- Find the `listen-address` , then add the IP of your DNS server
For example, my DNS server will bind at 10.0.2.101, port 53.

```
listen-address=10.0.2.101
```
In addition, you need to add the IP in `/etc/resolv.conf`


```
nameserver 10.0.2.101
```

Finally, add NAT rule for enabling DNS

```
$ sudo iptables -t nat -A PREROUTING -p udp --dport 53 -j DNAT --to-destination <IP of DNS server>
```