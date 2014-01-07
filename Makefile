PRODUCT = boot_ctrl
SERIAL_EMU = serial_emu

CC      = gcc
OBJCOPY = objcopy
LD      = ld

DEFS = -DTP01

CFLAGS = -O0 -g
CFLAGS += -Wall -Wextra -Warray-bounds

LDFLAGS =

CTRL_OBJS = main.o utils.o bl_proto.o
EMU_OBJS = test/emu.o
RS232_OBJS = rs232.o

all: $(PRODUCT)

$(PRODUCT): $(CTRL_OBJS) $(RS232_OBJS)
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS) $(CTRL_OBJS) $(RS232_OBJS) -o $@

$(SERIAL_EMU): $(SERIAL_EMU_OBJS) $(RS232_OBJS)
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS) $(EMU_OBJS) $(RS232_OBJS) -o $@ -lutil 

clean:
	rm -f *.o $(PRODUCT)
