from socket import *
import time
import random

settings = {
    'HOST'           : '1.239.197.74', # default
    'PORT'           : 5005,
}

sock = socket(AF_INET, SOCK_DGRAM)

while True:
    # tmp = random.randrange(-400, 400)
    tmp = 200
    print(tmp)
    sock.sendto(str(tmp).encode(), (settings['HOST'], settings['PORT']))
    time.sleep(0.5)