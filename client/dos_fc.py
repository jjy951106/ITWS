from socket import *
import time
import random

settings = {
    'HOST'           : '1.239.197.74', # default
    'PORT'           : 5005,
    'on'             : True,
}

sock = socket(AF_INET, SOCK_DGRAM)

while True:
    if settings['on'] == True:
        tmp = 'offset'
        settings['on'] = False
    else:
        # tmp = random.randrange(-400, 400)
        tmp = random.randrange(0, 40)
        settings['on'] = True
    print(tmp)
    sock.sendto(str(tmp).encode(), (settings['HOST'], settings['PORT']))
    time.sleep(0.005)
    