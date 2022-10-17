import struct
import sys
import socket
import binascii

def rfc1071_chksum(data):
    s = 0       # Binary Sum

    # loop taking 2 characters at a time
    for i in range(0, len(data), 2):
        if (i+1) < len(data):
            a = (data[i]) 
            b = (data[i+1])
            s = s + (a+(b << 8))
        elif (i+1)==len(data):
            s += (data[i])
        else:
            raise "Something Wrong here"


    # One's Complement
    s = s + (s >> 16)
    s = ~s & 0xffff

    return s        

def ip_unpack(data):
        vihl, tos, total_len, identification, flags_fragment, TTL, proto, header_checksum, s_ip, d_ip = struct.unpack('! B B H H H B B H 4s 4s', data[:20])
        
        version = vihl >> 4
        ihl = vihl & 0xF

        if ihl > 5:
                print("Support for IPv4 Options in unpack not available")
                sys.exit(-1)
        else:
                return version, ihl, tos, total_len, identification, flags_fragment, TTL, proto, header_checksum , getip(s_ip), getip(d_ip), data[20:]

def ip_header_pack(version, ihl, tos, total_len, identification, flags_fragment, TTL, proto, header_checksum , s_ip, d_ip):
        ip_ihl_ver = (version << 4) + ihl

        return struct.pack('!BBHHHBBH4s4s' , ip_ihl_ver, tos, total_len, identification, flags_fragment, TTL, proto, header_checksum, s_ip, d_ip)

def getip(ip_bytes):
        return '.'.join(map(str, ip_bytes))

def gtp_unpack(data):
    vih, message_type, message_len, teid = struct.unpack('! B B H I',data[:8])
    
    version = vih>>5
    protocol_type = (vih >> 4) & 1
    extension_header_flg = (vih >> 2) & 1
    seq_number_flg = (vih >> 1) & 1
    npdu_number_flg = vih & 1

    if(extension_header_flg or seq_number_flg or npdu_number_flg):
        seq_number, npdu_number, next_extension_header_flg = struct.unpack('! H B B',data[8:11])
        return version, protocol_type, extension_header_flg, seq_number_flg, npdu_number_flg, message_type, message_len, teid, data[12:]
    else:
        return version, protocol_type, extension_header_flg, seq_number_flg, npdu_number_flg, message_type, message_len, teid, data[8:]

def generate_general_pdn_gtp_header(data_len,teid):
        return struct.pack("!BBHI",0x30,0xFF,data_len,teid)

def ip_packet_mod_source_params(ip_data, new_src_ip, new_src_port, ip_header = False):

        version, ihl, tos, total_len, identification, flags_fragment, TTL, proto, header_checksum , src_ip, dst_ip, tcp_payload = ip_unpack(ip_data)

        # print("Original Checksums IP{} TCP{}".format(hex(struct.unpack("H",ip_data[10:12])[0]), hex(struct.unpack("H",tcp_payload[16:18])[0])))
        # replace source ip with new value and clear checksum field
        s_changed_ip_header = ip_header_pack(version, ihl, tos, total_len, identification, 
                                flags_fragment, TTL, proto, 0 , socket.inet_aton(new_src_ip), socket.inet_aton(dst_ip)) 
        if proto == 6:
                """ fetch values for checksum calculation and to return to main called about the original address information """
                orig_src_ip = src_ip
                orig_src_port = struct.unpack("!H",tcp_payload[:2])[0]
                final_dst_ip = dst_ip
                final_dst_port = struct.unpack("!H",tcp_payload[2:4])[0]  

                """ start building the new TCP packet """
                # replace source port with new value
                new_packet = struct.pack("!H",new_src_port) + tcp_payload[2:]
                # clear tcp checksum field
                new_packet = new_packet[:16] + struct.pack("!H",0) + new_packet[18:]
                src_ip_aton = socket.inet_aton(new_src_ip)
                dst_ip_aton = socket.inet_aton(final_dst_ip)
                # build pseudo header
                pseudo_header = struct.pack("!4s4sHH", src_ip_aton, dst_ip_aton, socket.IPPROTO_TCP, len(new_packet))
                # calculate checksum of TCP packet
                _checksum = rfc1071_chksum(pseudo_header + new_packet)
                # write checksum to packet
                new_packet = new_packet[:16] + struct.pack("H",_checksum) + new_packet[18:]

                if ip_header:
                        # calculate checksum of IP header
                        _checksum = rfc1071_chksum(s_changed_ip_header)
                        # write checksum to packet
                        s_changed_ip_header = s_changed_ip_header[:10] + struct.pack("H",_checksum) + s_changed_ip_header[12:]
                        # add IP header and TCP packet if IP header is requested
                        new_packet = s_changed_ip_header + new_packet
                        # print("Original Checksums IP{} TCP{}".format(hex(struct.unpack("H",new_packet[10:12])[0]), hex(struct.unpack("H",new_packet[36:38])[0])))

        elif proto == 17:
                print("UDP not yet implemented")
                sys.exit(-1)                        

        
        return new_packet, proto, orig_src_ip, orig_src_port, final_dst_ip, final_dst_port      

def ip_packet_mod_destination_params(ip_data, new_dst_ip, new_dst_port, ip_header = True):

        version, ihl, tos, total_len, identification, flags_fragment, TTL, proto, header_checksum , src_ip, dst_ip, tcp_payload = ip_unpack(ip_data)

        # print("Original Checksums IP{} TCP{}".format(hex(struct.unpack("H",ip_data[10:12])[0]), hex(struct.unpack("H",tcp_payload[16:18])[0])))
        # replace destination ip with new value and clear checksum field
        d_changed_ip_header = ip_header_pack(version, ihl, tos, total_len, identification, 
                                flags_fragment, TTL, proto, 0 , socket.inet_aton(src_ip), socket.inet_aton(new_dst_ip)) 
        if proto == 6:
                """ fetch values for checksum calculation and to return to main called about the original address information """
                start_src_ip = src_ip
                start_src_port = struct.unpack("!H",tcp_payload[:2])[0]
                orig_dst_ip = dst_ip
                orig_dst_port = struct.unpack("!H",tcp_payload[2:4])[0]  

                """ start building the new TCP packet """
                # replace destinatn port with new value
                new_packet = tcp_payload[:2] + struct.pack("!H",new_dst_port) + tcp_payload[4:]
                # clear tcp checksum field
                new_packet = new_packet[:16] + struct.pack("!H",0) + new_packet[18:]
                src_ip_aton = socket.inet_aton(start_src_ip)
                dst_ip_aton = socket.inet_aton(new_dst_ip)
                # build pseudo header
                pseudo_header = struct.pack("!4s4sHH", src_ip_aton, dst_ip_aton, socket.IPPROTO_TCP, len(new_packet))
                # calculate checksum of TCP packet
                _checksum = rfc1071_chksum(pseudo_header + new_packet)
                # write checksum to packet
                new_packet = new_packet[:16] + struct.pack("H",_checksum) + new_packet[18:]

                if ip_header:
                        # calculate checksum of IP header
                        _checksum = rfc1071_chksum(d_changed_ip_header)
                        # write checksum to packet
                        d_changed_ip_header = d_changed_ip_header[:10] + struct.pack("H",_checksum) + d_changed_ip_header[12:]
                        # add IP header and TCP packet if IP header is requested
                        new_packet = d_changed_ip_header + new_packet
                        # print("Original Checksums IP{} TCP{}".format(hex(struct.unpack("H",new_packet[10:12])[0]), hex(struct.unpack("H",new_packet[36:38])[0])))

        elif proto == 17:
                print("UDP not yet implemented")
                sys.exit(-1)                        

        return new_packet, proto, start_src_ip, start_src_port, orig_dst_ip, orig_dst_port           


        


