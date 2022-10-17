import socket
import time
from data import *


# change this based on the system IP used like AWS server or the Hardware LTE setups
HOST = '10.0.0.1'    	 
PORT = 7891

data_handle = data_handler()
recv_counter = 0

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen()
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        while True:
            data = conn.recv(1500)
            if not data:
                break
            #conn.sendall(data)
            conn.sendall(data_handle.reply_dsrc_packet(data,10))
            print("SND>RCV ",recv_counter)
            recv_counter += 1 
