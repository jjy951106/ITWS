from socket import *
import paho.mqtt.client as mqtt
from pymavlink import mavutil
from datetime import datetime as dt
import time
from socket import *

from MAVProxy.modules.lib import mp_module
from MAVProxy.modules.mavproxy_timesync import *

HOST = "1.239.197.74" # 1.239.197.74
PORT = 5005
ADDR = (HOST, PORT)
connection = False
connectionIndex = 1

count = tmp = fc_lt = 0
N = 10 # this is needed to reduce
sendTerm = 5 # second

sock = socket(AF_INET, SOCK_DGRAM)

while(connection is False):
    try:
        if connectionIndex == 1:
            print('1')
            fc_port = mavutil.mavlink_connection("/dev/ttyACM0") # /dev/ttyACM0 or /dev/ttyACM1
        elif connectionIndex == 2:
            print('2')
            fc_port = mavutil.mavlink_connection("/dev/ttyACM1")
        elif connectionIndex == 3:
            print('3')
            fc_port = mavutil.mavlink_connection("/dev/ttyAMA0")
        elif connectionIndex == 4:
            print('4')
            fc_port = mavutil.mavlink_connection("/dev/serial0") # USB0
        elif connectionIndex == 5:
            print('5')
            fc_port = mavutil.mavlink_connection("/dev/serial1") # USB0
        else:
            print('6')
            fc_port = mavutil.mavlink_connection("COM6") # USB1
        connection = True
    except:
        connectionIndex = connectionIndex + 1
        pass

fc_port.mav.param_set_send( fc_port.target_system, fc_port.target_component, b'BRD_RTC_TYPES', 3, mavutil.mavlink.MAV_PARAM_TYPE_INT32 ) # Ardupilot

fc_port.mav.param_request_read_send( fc_port.target_system, fc_port.target_component, b'BRD_RTC_TYPES', -1 )
time.sleep(2)

try:
    message = fc_port.recv_match(type='PARAM_VALUE', blocking=True).to_dict()
    print(message['param_id'], message['param_value'])
except Exception as e:
    print(e)
    exit(0)

# Interval initialize
fc_port.mav.request_data_stream_send( fc_port.target_system, fc_port.target_system, 0, 2, 1 ) # high rate is fast and means 1/second

# Set FC time
while True:

    fc_port.mav.system_time_send( int(time.time() * 1e6) , 0 )
    
    # print(msg.get_type())
    # print(int(time.time() * 1e6)) # us
    # print(int(time.time() * 1e9)) # ns
    
    msg = fc_port.recv_match(type='SYSTEM_TIME',blocking=True)
    
    print(f'msg_time : {msg.time_unix_usec}')
  
    if msg.time_unix_usec > 10: break    

start = time.time()

while True:

    # Send timesync
    tx_time = dt.timestamp(dt.now())
    
    print(tx_time)
    
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
    now = float( dt.timestamp( dt.now() ) - fc_port.time_since('SYSTEM_TIME') )
    fc_time = float( msg.time_unix_usec / 1e6 )
    fc_offset = int( ( (fc_time + fc_lt) - now ) * 1000 )
    
    print("msg : {}\nfc_time : {}\nfc_offset : {}\nnow : {}".format(msg, fc_time, fc_offset, now))
    
    # send ms measure
    count = count + 1
    tmp = tmp + (fc_offset / N)
    if count is N:
        enteredTime = time.time() - start
        if sendTerm - enteredTime >= 0:
            time.sleep(sendTerm - enteredTime)
        
        # more than 1s companste gps time assumes gps sync problem and so this problem is ignored.
        if abs(tmp) < 1000:
            sock.sendto(str(tmp).encode(), ADDR)
        count = 0
        tmp = 0
        
        # startTime initialization
        start = time.time()

        print(f'enteredTime : {enteredTime}, sleepTime : {sendTerm - enteredTime}')
