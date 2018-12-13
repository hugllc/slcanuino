PLATFORMIO = platformio
PORT_NAME  = ttyACM0
PORT       = /dev/$(PORT_NAME)

RM         = rm -Rf


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
	sudo slcan_attach -f -s6 -o $(PORT)
	sudo slcand -S 1000000 $(PORT_NAME) can0
	sudo ip link set up can0

detach:
	sudo ip link set down can0
	sudo killall slcand

clean:
	${RM} .piolibdeps .pioenvs
