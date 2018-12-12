#!/bin/sh -e

# This is a script used for testing.  It will also start and stop the port.
#
#

PORT=/dev/ttyACM0
BAUD=1000000
DEV=slcan0

set -e

case "$1" in
  start)
        echo "Attempting to start ${DEV}"
        slcan_attach -f -s4 -o ${PORT}
        slcand -S ${BAUD} ${PORT} ${DEV}  
        ifconfig ${DEV} up  
        exit 0
        ;;
    stop)
        echo "Shutting down can0"
        ifconfig can0 down
        killall slcand
        exit 0
        ;;
    monitor)
        stty raw -echo ${BAUD} < ${PORT}; cat -vte ${PORT}
        stty raw -echo 115200 < ${PORT}
        ;;
    setbaud)
        echo "Sending S4"
        echo -n 'S4' > ${PORT}
        echo -n '\r' > ${PORT}
        ;;
    timestamp)
        echo "Sending Z"
        echo -n 'Z' > ${PORT}
        echo -n '\r' > ${PORT}
        ;;
    open)
        echo "Sending O"
        echo -n 'O' > ${PORT}
        echo -n '\r' > ${PORT}
        ;;
    close)
        echo "Sending C"
        echo -n 'C' > ${PORT}
        echo -n '\r' > ${PORT}
        ;;
    flag)
        echo "Sending F"
        echo -n 'F' > ${PORT}
        echo -n '\r' > ${PORT}
        ;;
    version)
        echo "Sending V"
        echo -n 'V' > ${PORT}
        echo -n '\r' > ${PORT}
        ;;
    serialnumber)
        echo "Sending N"
        echo -n 'N' > ${PORT}
        echo -n '\r' > ${PORT}
        ;;
    powerup)
        echo "Sending N"
        echo -n 'T8D008000800030100000000001AEF' > ${PORT}
        echo -n '\r' > ${PORT}
        ;;
    *)
  echo "Usage: /etc/init.d/<your script> {start|stop}"
    exit 1
    ;;
esac

