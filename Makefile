ARDUINO = ~/Documents/arduino/arduino
BOARD   = arduino:avr:uno
PORT_NAME    = ttyACM0
PORT    = /dev/$(PORT_NAME)
RM      = rm -f

FIRMWARE = slcan.ino

.PHONY: verify upload clean

default: verify

verify:
	$(ARDUINO) --board $(BOARD) --verify $(FIRMWARE)

upload:
	$(ARDUINO) --board $(BOARD) --port $(PORT) --upload $(FIRMWARE)

attach:
	sudo slcan_attach -f -s6 -o $(PORT)
	sudo slcand -S 1000000 $(PORT_NAME) can0
	sudo ip link set up can0

detach:
	sudo ip link set down can0
	sudo killall slcand
