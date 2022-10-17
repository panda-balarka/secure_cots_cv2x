import sys
import socket
import binascii
import data_handler
import struct

ip_lst = ["10.0.0.1"]

def gtp_dispatcher(mec_ip="172.17.1.2", dispatcher_port=7000, spgw_ip="172.17.1.4", spgw_udp_port=2152, vif_ip="172.17.1.2", vif_port=7001):
    
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((mec_ip, dispatcher_port))    
    d = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    ul = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_TCP)
    ul.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1)

    print("MEC_IP/DISPATCHER_PORT {}:{}. SPGW_IP/UDP_DATA_PORT {}:{}".format(mec_ip,dispatcher_port,spgw_ip,spgw_udp_port))

    while True:
        data,_ = s.recvfrom(1500)

        """ GTP handling needs to change if we are looking at non ip protocols and 
            MTUs greater than 1500 (MTU>1500 may require sequence and extension flags) """
        _, _, _, _, _, _, _, _, ip_data = data_handler.gtp_unpack(data)
        _, _, _, _, _, _, _,proto,_,_,d_ip,payload = data_handler.ip_unpack(ip_data)

        if d_ip in ip_lst:
            print(">M")
            if proto == 6:
                ul.sendto(ip_data,(d_ip, 0))
        else:
            print(">C")
            d.sendto(data,(spgw_ip,spgw_udp_port))

         
if __name__ == "__main__":
    # args: script MEC_IP DISPATCH_PORT SPGW_IP SPGW_UDP_PORT VIRTUAL_IF_IP VIRTUAL_IF_PORT
    if len(sys.argv) == 1:
        gtp_dispatcher()
    elif len(sys.argv) == 5:       
        gtp_dispatcher(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
    elif len(sys.argv) == 6:
        gtp_dispatcher(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
    else:
        # TODO: Update this to show help and script usage
        print("Invalid number of arguments\n")
