
#
#	Makefile for time project
#

DEBUG = -g
CC = qcc
LD = qcc

TARGET = -Vgcc_ntox86_64
#TARGET = -Vgcc_ntoaarch64le

CFLAGS += $(DEBUG) $(TARGET) -Wall
LDFLAGS+= $(DEBUG) $(TARGET)

BINS = reptimer nonblockpulserec

all: $(BINS)

clean:
	rm -f *.o $(BINS)
	cd solutions && make clean_soln

