import ctypes
import time
import os
import datetime
import random

key_dict = {
        0 : [ 0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02, 
              0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02],
        1 : [ 0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02, 
              0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03,0x02,0x02,0x02,0x02],
        2 : [ 0x04,0x04,0x04,0x04,0x02,0x02,0x02,0x02,0x04,0x04,0x04,0x04,0x02,0x02,0x02,0x02, 
              0x04,0x04,0x04,0x04,0x02,0x02,0x02,0x02,0x04,0x04,0x04,0x04,0x02,0x02,0x02,0x02],
}

lat = [0x235A,0x1234,0x5678,0x4537,0x1356,0x5677,0x4568,0x2356,0xDEAF,0xABCD,0x12AB,0x98BC]
lon = [0x4664,0x4321,0x8765,0x7354,0x6531,0x7765,0x8654,0x6532,0xFAED,0xDCBA,0xBA21,0xCB89]
alt = [0xF000,0xE000,0xD000,0xC000,0x1000,0x2000,0x3000,0x4000,0x5000,0x6000,0x7000,0x8000]

class data_handler(object):

    reply_ctr = 1
    total_exec_time = 0

    def __init__(self):
        self.dsrc_lib = ctypes.CDLL(os.path.join(os.path.dirname(os.path.realpath(__file__)),"dsrc.so"))
        self.ascon_lib = ctypes.CDLL(os.path.join(os.path.dirname(os.path.realpath(__file__)),"hmac.so"))

    def dsrc_encode(self, msg_cnt=00, lat=0xFFFFFFFFF, long=0xFFFFFFFF, elev=0xFFFF):
        
        self.dsrc_lib.generateBSM_data_verbose_DSRC.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint16]
        asn_buff_p = (ctypes.c_uint8*100)()
        asn_buff_size_p = (ctypes.c_uint8)()

        returnVal = self.dsrc_lib.generateBSM_data_verbose_DSRC(asn_buff_p, asn_buff_size_p, msg_cnt, lat, long, elev)  

        data_lst = (list(map(lambda x: format(x, '#04x')[2:],list(asn_buff_p)))[:asn_buff_size_p.value])
        hex_str = ''.join(data_lst)
        
        return hex_str, bytes.fromhex(hex_str)
        
    def ascon_hmac(self, byte_arr, ip_size, key_idx):

        self.ascon_lib.crypto_hmac.argtypes = [ctypes.POINTER(ctypes.c_uint8), 
                                            ctypes.POINTER(ctypes.c_uint8), ctypes.c_uint64,
                                            ctypes.POINTER(ctypes.c_uint8)]
        out_buff_p = (ctypes.c_uint8*32)()

        key_arr = bytes.fromhex(''.join(list(map(lambda x: format(x, '#04x')[2:],list(key_dict[key_idx])))))
        returnVal = self.ascon_lib.crypto_hmac(out_buff_p, 
                            ctypes.cast(ctypes.c_char_p(byte_arr),ctypes.POINTER(ctypes.c_uint8)), ip_size,
                            ctypes.cast(ctypes.c_char_p(key_arr),ctypes.POINTER(ctypes.c_uint8)))
        
        data_lst = list(map(lambda x: format(x, '#04x')[2:],list(out_buff_p)))
        hex_str = ''.join(data_lst)

        return hex_str

    def build_dsrc_v_data_packet(self, imei = "FFFFFFFFFFFFFFFF", lat=0xFFFFFFFFF, long=0xFFFFFFFF, elev=0xFFFF):
        starttime = time.time()    
        # find relevant data entries
        key_id = random.randint(0,2)
        header_str = '03' + str(key_id).zfill(2) + imei.zfill(16)
        # build dsrc data
        hex_str, hex_bytes = self.dsrc_encode(data_handler.reply_ctr%256, lat, long, elev)
        # 4 bytes time nonce month, day, hour, min
        nonce_time = datetime.datetime.now()
        nonce_str = "{:02x}{:06x}".format(nonce_time.second,nonce_time.microsecond)
        # call ascon hmac function
        hex_str = header_str + hex(int(len(hex_str)/2)).strip('0x') + hex_str + nonce_str
        hmac_str = self.ascon_hmac(bytes.fromhex(hex_str), int(len(hex_str)/2), key_id)
        # call the appender function
        hex_str += hmac_str
        endtime= time.time()
        #print("Time in microseconds \n", (endtime-starttime)*1000000)
        return hex_str

    def reply_dsrc_packet(self, received_msg, recv_packets=1):
        start_time = time.time()
        imei_rcv = received_msg.hex()[4:20]
        #print("Received :",received_msg)
        resp = hex(recv_packets).strip('0x').zfill(2)
        for i in range(0,recv_packets):
            resp += self.build_dsrc_v_data_packet(imei_rcv,random.choice(lat), random.choice(lon), random.choice(alt))
        data_handler.total_exec_time += (time.time()-start_time)
        #print(resp)
        if(data_handler.reply_ctr%1000 == 0):
            print("Server overhead: ",(data_handler.total_exec_time/data_handler.reply_ctr))
        data_handler.reply_ctr += 1        
        return bytes.fromhex(resp)

    def hex_str_to_c_array(self, hex_str):
        temp_str = ""
        for i in range(0,len(hex_str),2):
            if (i!=0 and i%32==0):
                temp_str +="\n"        
            temp_str += "0x"+hex_str[i:i+2]+", "
        print(temp_str)

    def byte_to_hex_array(self,ip):
        print(ip.hex())

if __name__ == "__main__":
    data_obj = data_handler()
    data_obj.reply_dsrc_packet(b'\x03\x02\x08`\x19PU\x82I\x05B0@\x80\x01\x03\x81\x01\x01\x82\x04V4\x12e\x83\x01\x01\x84\x02Z#\x85\x02dF\x86\x02\x00\xf0\x87\x04\xff\xff\x01\xfe\x88\x02\x10\xfe\x89\x01@\x8a\x01\x01\x8b\x07\x01#E`\x124V\x8c\x02\xff\xff\xad\x06\x80\x01\x01\x81\x01\x05\xfa\xf9\x0f\xdf\xb2`|\xea\x86\xf3\xa3\r\n\xcc\'\xcb<\xa0\x8a\xde\xc3\xce\xdapM\x7f"\xff\xb5\xd5a\x93O\xf6x\xaa\x03\x00\x08`\x19PU\x82I\x05B0@\x80\x01\x03\x81\x01\x01\x82\x04V4\x12e\x83\x01\x01\x84\x02Z#\x85\x02dF\x86\x02\x00\xf0\x87\x04\xff\xff\x01\xfe\x88\x02\x10\xfe\x89\x01@\x8a\x01\x01\x8b\x07\x01#E`\x124V\x8c\x02\xff\xff\xad\x06\x80\x01\x01\x81\x01\x05\xf8\x984\xdfzW\xe8\xb1hHV\x9aw\x80.\xe7\x93\xbd\xa6S4\xab\x1f\xa0\xe4\xcd|\x864\x9c\xc0\\5\xff6t\x03\x01\x08`\x19PU\x82I\x05B0@\x80\x01\x03\x81\x01\x01\x82\x04V4\x12e\x83\x01\x01\x84\x02Z#\x85\x02dF\x86\x02\x00\xf0\x87\x04\xff\xff\x01\xfe\x88\x02\x10\xfe\x89\x01@\x8a\x01\x01\x8b\x07\x01#E`\x124V\x8c\x02\xff\xff\xad\x06\x80\x01\x01\x81\x01\x05\xf37Y\xdf\xfe3\x05\x0b\x9a\xach\xc6\xea\xfa\xb0\x85I\x17,z\x89vF\xb7\x15\xac\xe2\x9et\x07>\xb1\xf1\x80\xde\x0f\x03\x02\x08`\x19PU\x82I\x05B0@\x80\x01\x03\x81\x01\x01\x82\x04V4\x12e\x83\x01\x01\x84\x02Z#\x85\x02dF\x86\x02\x00\xf0\x87\x04\xff\xff\x01\xfe\x88\x02\x10\xfe\x89\x01@\x8a\x01\x01\x8b\x07\x01#E`\x124V\x8c\x02\xff\xff\xad\x06\x80\x01\x01\x81\x01\x05\xef\xd6}\xdf\xcf\x1aH\x0eJ\x87\x83_\xaf\xd4\xa2\xd3\xb4v\xa8\xd1F\xcb\xe2\xe5n52\xd4\x99E\x16\xb8\x88\xcaC\x1f',10)
    s = b"\x03\x00\x08`\x19PU\x82I\x05B0@\x80\x01\x03\x81\x01\x01\x82\x04V4\x12e\x83\x01\x01\x84\x02Z#\x85\x02dF\x86\x02\x00\xf0\x87\x04\xff\xff\x01\xfe\x88\x02\x10\xfe\x89\x01@\x8a\x01\x01\x8b\x07\x01#E`\x124V\x8c\x02\xff\xff\xad\x06\x80\x01\x01\x81\x01\x05\x9e\xb2T\xd2f\x1d\xa2\xea,}\x95;\x99\xc8(l\x82\x17\x91\x047\xa1\x96\xe6\xfc\xa8X\xaa\xe6\x00I\xb1i\xa7\x0ed\x03\x01\x08`\x19PU\x82I\x05B0@\x80\x01\x03\x81\x01\x01\x82\x04V4\x12e\x83\x01\x01\x84\x02Z#\x85\x02dF\x86\x02\x00\xf0\x87\x04\xff\xff\x01\xfe\x88\x02\x10\xfe\x89\x01@\x8a\x01\x01\x8b\x07\x01#E`\x124V\x8c\x02\xff\xff\xad\x06\x80\x01\x01\x81\x01\x05\x9fQy\xd2K\x81\x87\xf8t\x06\xa0|\xef\xc0\xe0:]PC\xfa\x1a_p\x98\xf4z\xbb\x8ap,\xcfXG_+\xe3\x03\x02\x08`\x19PU\x82I\x05B0@\x80\x01\x03\x81\x01\x01\x82\x04V4\x12e\x83\x01\x01\x84\x02Z#\x85\x02dF\x86\x02\x00\xf0\x87\x04\xff\xff\x01\xfe\x88\x02\x10\xfe\x89\x01@\x8a\x01\x01\x8b\x07\x01#E`\x124V\x8c\x02\xff\xff\xad\x06\x80\x01\x01\x81\x01\x05\x93\xf0\x9d\xd2URI\xbe1E\x07l\x8dO\xc7\x90\x04#c\xf4\xf8\xb4I\xe9\x0f\xbf\xea\x10Tj\xeaqR\x81Z\xe1"
    print(s.hex())
    #hex_str_to_c_array("010300086019505582490542304080010381010182045634126583010184025a2385026446860200f08704ffff01fe880210fe8901408a01018b07012345601234568c02ffffad068001018101050c0763eac9a7e0cd78a27d023ac7bc84c8f710c65e57742a61c32d180eb56d8b080c843b")    
    #byte_to_hex_array(b'\x03\x00\x08`\x19PU\x82I\x05B0@\x80\x01\x03\x81\x01\x01\x82\x04V4\x12e\x83\x01\x01\x84\x02Z#\x85\x02dF\x86\x02\x00\xf0\x87\x04\xff\xff\x01\xfe\x88\x02\x10\xfe\x89\x01@\x8a\x01\x01\x8b\x07\x01#E`\x124V\x8c\x02\xff\xff\xad\x06\x80\x01\x01\x81\x01\x05\xa7T\x0e7\xd0\xbc\x1c6g3\xd8\xe8\x98z\xdb\x17\xe0\xefJ"\x9en\xce\xbb/\xa2\xbb\x9b\xc73[\x9f6\x0eV\xf5')



