PLATFORMIO  = platformio
PORT_NAME  ?= ttyACM0
PORT        = /dev/$(PORT_NAME)
DEV        ?= can0
RM          = rm -Rf


.PHONY: verify upload clean

default: verify

verify:
	$(PLATFORMIO) run

upload:
	$(PLATFORMIO) run -t upload --upload-port ${PORT}

update:
	${PLATFORMIO} platform update
	${PLATFORMIO} lib update

attach:
	sudo slcan_attach -f -s4 -o $(PORT)
	sudo slcand -S 1000000 $(PORT_NAME) ${DEV}
	sudo ip link set up ${DEV}

detach:
	sudo ip link set down ${DEV}
	sudo killall slcand

clean:
	${RM} .piolibdeps .pioenvs
