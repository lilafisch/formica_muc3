# Copyright 2008 Stephen English, Jeffrey Gough, Alexis Johnson, 
#     Robert Spanton and Joanna A. Sun.

# This file is part of the Formica robot firmware.

# The Formica robot firmware is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# The Formica robot firmware is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with the Formica robot firmware.  
# If not, see <http://www.gnu.org/licenses/>.

ARCH=msp430x2274
CFLAGS := -mmcu=$(ARCH) -g -Wall -Os
CFLAGS += -mendup-at=main
LDFLAGS :=

CC := msp430-gcc

C_FILES := main.c ir-rx.c freq.c net-rx.c opamp-1.c ir-tx.c \
	ir-tx-data.c net-tx.c adc10.c random.c motor.c virus.c \
	smbus_pec.c battery.c ir.c food.c bearing.c flash.c \
	behav/braitenberg.c time.c behav/parking.c behav/watchdog.c leds.c
H_FILES := device.h ir-rx.h freq.h net-rx.h opamp-1.h ir-bias.h \
	ir-tx.h ir-tx-data.h net-tx.h adc10.h random.h motor.h net.h \
	leds.h virus.h smbus_pec.h battery.h ir.h food.h bearing.h flash.h \
	behav/braitenberg.h time.h behav/parking.h behav/watchdog.h

include .config

world: main main-top

ifeq ($(strip $(CONF_TX_SEQ)),y)
CFLAGS += -DCONF_TX_SEQ=1
endif

CFLAGS += -DRAND_WALK_SPEED=$(RAND_WALK_SPEED)

ifneq ($(strip $(WIN)),y)
# Number of symbols we have to transmit per byte
SYM_PER_BYTE := `./.sym_per_bit.py $(NBITS)`

world: main .freq.h.win .freq.c.win
else
SYM_PER_BYTE := 3
endif

# Get a new firmware revision number!
FW_VER = `cat .fw_ver`
CFLAGS += -DFW_VER=`cat .fw_ver`

.fw_ver: $(C_FILES) $(H_FILES) lkr/$(ARCH)-lower.x lkr/$(ARCH)-upper.x
ifeq ($(strip $(FW)),)
	curl -s http://users.ecs.soton.ac.uk/rds204/formica/rev.php > .fw_ver
else
	echo $(FW) > .fw_ver
endif

main: $(C_FILES) $(H_FILES) lkr/$(ARCH)-lower.x .fw_ver
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(C_FILES) -Wl,-T,lkr/$(ARCH)-lower.x
	@echo Firmware revision $(FW_VER)

main-top: $(C_FILES) $(H_FILES) lkr/$(ARCH)-upper.x .fw_ver
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(C_FILES) -Wl,-T,lkr/$(ARCH)-upper.x
	@echo Firmware revision $(FW_VER)

ifneq ($(strip $(WIN)),y)
freq.c: freq.py .config
	./freq.py $(MIN_PERIOD) $(MAX_PERIOD) $(NBITS) > freq.c

freq.h: .freq.h .config
	cp .freq.h freq.h
	sed -i -e "s/_NFREQ/$(NFREQ)/" freq.h
	sed -i -e "s/_NBITS/$(NBITS)/" freq.h
	sed -i -e "s/_MIN_PERIOD/$(MIN_PERIOD)/" freq.h
	sed -i -e "s/_MAX_PERIOD/$(MAX_PERIOD)/" freq.h
	sed -i -e "s/_SYMBOLS_PER_BYTE/$(SYM_PER_BYTE)/" freq.h

.freq.c.win: freq.c
	cp -f freq.c .freq.c.win

.freq.h.win: freq.h
	cp -f freq.h .freq.h.win
else
# Alexis's windows mode
freq.c: .freq.c.win
	cp .freq.c.win freq.c

freq.h: .freq.h.win
	cp .freq.h.win freq.h
endif

lkr/$(ARCH)-norm.x:
	mkdir -p lkr
	msp430-ld -m $(ARCH) --verbose | csplit -f ./lkr/$(ARCH)-norm- - "%==================================================%1"
	sed -i ./lkr/$(ARCH)-norm-00 -e 's/==================================================//'
	mv ./lkr/$(ARCH)-norm-00 ./lkr/$(ARCH)-norm.x

lkr/$(ARCH)-upper.x: lkr/$(ARCH)-norm.x
	cat $< | sed -e 's/^[[:space:]]*text[[:space:]]*(rx).*$$/text : ORIGIN = 0xe000, LENGTH = 0x1e00/' > $@

lkr/$(ARCH)-lower.x: lkr/$(ARCH)-norm.x
	cat $< | sed -e 's/^[[:space:]]*text[[:space:]]*(rx).*$$/text : ORIGIN = 0xc000, LENGTH = 0x1e00/' > $@

.PHONY: clean

clean: 
	-rm -f main main-top freq.{h,c} .fw_ver
	-rm -f lkr/$(ARCH)-{upper,lower,norm}.x

