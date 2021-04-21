from socket import *
import paho.mqtt.client as mqtt
from pymavlink import mavutil
from datetime import datetime as dt

HOST = "1.239.197.74" # 1.239.197.74
PORT = 5005
ADDR = (HOST, PORT)

count = tmp = fc_lt = 0
N = 20

sock = socket(AF_INET, SOCK_DGRAM)

fc_port = mavutil.mavlink_connection("/dev/ttyACM1") # /dev/ttyACM1

# Interval initialize
fc_port.mav.request_data_stream_send( fc_port.target_system, fc_port.target_system, 0, 5, 1 )            

# fc_port.mav.param_set_send('BRD_RTC_TYPES', 2) # Ardupilot

# Set FC time
while True:

    fc_port.mav.system_time_send( int( dt.timestamp(dt.now()) * 1e6 ) , 0 )
    msg = fc_port.recv_match(type='SYSTEM_TIME',blocking=True)
    # print(msg.time_unix_usec)
    if msg.time_unix_usec > 10: break    
   
while True:

    # Send timesync
    tx_time = dt.timestamp(dt.now())
    fc_port.mav.timesync_send(0, int( tx_time ))

    # Time sync message reception
    msg = fc_port.recv_match(type='TIMESYNC',blocking=True)
    if msg.tc1 == 0:
        continue
    else:
        rx_time = dt.timestamp(dt.now())
        if fc_lt != 0: fc_lt = (fc_lt + (rx_time - tx_time) / 2 ) / 2
        else: fc_lt = (rx_time - tx_time) / 2

    print("fc_lt : {}, rx_time : {}, tx_time : {}".format(fc_lt, rx_time, tx_time))

    # System time message reception
    msg = fc_port.recv_match(type='SYSTEM_TIME',blocking=True)
    now = float( dt.timestamp( dt.now() ) )
    fc_time = float( msg.time_unix_usec / 1e6 )
    fc_offset = int( ( (fc_time + fc_lt) - now ) * 1000 )
    
    print("msg : {}\nfc_time : {}\nfc_offset : {}\nnow : {}".format(msg, fc_time, fc_offset, now))
    
    # send ms measure
    count = count + 1
    tmp = tmp + (fc_offset / N)
    if count is N:
        sock.sendto(str(tmp).encode(), ADDR)
        count = 0
        tmp = 0