from socket import *
import paho.mqtt.client as mqtt
from pymavlink import mavutil
from datetime import datetime as dt
import time
from socket import *

settings = {
    'HOST'           : '1.239.197.74', # default
    'PORT'           : 5005,
    'Connection'     : False,
    'ConnectionLink' : ['/dev/ttyACM0', '/dev/ttyACM1', '/dev/ttyAMA0',\
                        '/dev/serial0', '/dev/serial1', 'COM6'],
    'DataRate'       : 2,
    'TransmitPacket' : 10,
    'SendTerm'       : 5,
    'BRD_RTC_TYPES'  : 3,              # GPS, MAVLINK
}

count = connectionIndex = tmp = fc_lt = 0

while(settings['Connection'] is False):
    try:
        fc_port = mavutil.mavlink_connection(settings['ConnectionLink'][connectionIndex])
        settings['Connection'] = True
        print('Success OpenLink {}'.format(settings['ConnectionLink'][connectionIndex]))
    except:
        connectionIndex = connectionIndex + 1
        if connectionIndex == len(settings['ConnectionLink']): connectionIndex = 0
        pass

fc_port.mav.param_set_send( fc_port.target_system, fc_port.target_component, b'BRD_RTC_TYPES',\
                            settings['BRD_RTC_TYPES'], mavutil.mavlink.MAV_PARAM_TYPE_INT32 ) # for Ardupilot

fc_port.mav.param_request_read_send( fc_port.target_system, fc_port.target_component, b'BRD_RTC_TYPES', -1 )
time.sleep(2)

try:
    message = fc_port.recv_match(type='PARAM_VALUE', blocking=True).to_dict()
    print(message['param_id'], message['param_value'])
except Exception as e:
    print(e)
    exit(0)

# Interval initialize
fc_port.mav.request_data_stream_send( fc_port.target_system, fc_port.target_system, 0, settings['DataRate'], 1 )

# Set FC time
while True:

    fc_port.mav.system_time_send( int(time.time() * 1e6) , 0 )
    msg = fc_port.recv_match(type='SYSTEM_TIME',blocking=True)
    print(f'msg_time : {msg.time_unix_usec}')
    if msg.time_unix_usec > 10: break    
    
sock = socket(AF_INET, SOCK_DGRAM)

start = time.time()

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
    # print("fc_lt : {}, rx_time : {}, tx_time : {}".format(fc_lt, rx_time, tx_time))

    # System time message reception
    msg = fc_port.recv_match(type='SYSTEM_TIME',blocking=True)
    now = float( dt.timestamp( dt.now() ) - fc_port.time_since('SYSTEM_TIME') )
    fc_time = float( msg.time_unix_usec / 1e6 )
    fc_offset = int( ( (fc_time + fc_lt) - now ) * 1000 )
    
    print("\n----------------------------------------------------------------------------")
    print("msg : {}\nfc_time : {}\nfc_offset : {}\nnow : {}".format(msg, fc_time, fc_offset, now))
    print("----------------------------------------------------------------------------\n")
    
    # send ms measure
    count = count + 1
    tmp = tmp + (fc_offset / settings['TransmitPacket'])
    if count is settings['TransmitPacket']:
        enteredTime = time.time() - start
        if settings['SendTerm'] - enteredTime >= 0:
            time.sleep(settings['SendTerm'] - enteredTime)
        
        # more than 200ms companste gps time assumes gps sync problem and so this problem is ignored.
        if abs(tmp) < 200:
            sock.sendto(str(tmp).encode(), (settings['HOST'], settings['PORT']))
        count = 0
        tmp = 0
        
        # startTime initialization
        start = time.time()

        print('(Transmission Packet, Interval) : ({}, {}s)\n(EnteredTime, SleepTime) : ({:.3f}, {:.3f})'\
              .format(settings['TransmitPacket'], settings['SendTerm'], enteredTime, settings['SendTerm'] - enteredTime))
