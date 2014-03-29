# Makefile for project $(NAME)
# Distribute under GPLv2, use with care.
#
# 2008-09-25, jw

NAME          = matrix8x8
CFILES        = blink.c # $(NAME).c eeprom.c i2c_slave_cb.c
CPU           = mega8
#PROG_HW       = stk200
PROG_HW       = usbasp
#PROG_SW       = avrdude
PROG_SW       = sudo avrdude

TOP_DIR       = .

include $(TOP_DIR)/avr_common.mk

distclean:: 
	rm -f download* ee_data.* 

## header file dependencies
#############################
include depend.mk
