# Slcanuino

This is an Arduino sketch which makes a CAN-BUS shield into a CAN-USB adapter for 
Linux SocketCAN(can-utils).   This requires the MCP_CAN library to be installed
to compile.

http://skpang.co.uk/catalog/arduino-canbus-shield-with-usd-card-holder-p-706.html

# Supported hardware

I tested this sketch on the following 'CAN-BUS Shield'.

https://www.sparkfun.com/products/10039


# How to use

Burn your Arduino with this and install can-utils for your linux environment in advance.

## Deps
1. slcan (kernel module)
2. SocketCAN (http://www.pengutronix.de/software/libsocketcan/)
3. can-utils (https://github.com/linux-can/can-utils)
4. MCP_CAN_lib (https://github.com/coryjfowler/MCP_CAN_lib)

## Setup

    $ sudo slcan_attach -f -s6 -o /dev/ttyACM0
    $ sudo slcand -S 1000000 ttyACM0 can0
    $ sudo ip link set up can0

then,

    $ candump can0

## Cleanup

    $ sudo ip link set down can0
    $ sudo killall slcand
