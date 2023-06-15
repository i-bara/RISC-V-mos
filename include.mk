# ENDIAN is either EL (little endian) or EB (big endian)
ENDIAN         := EL

ifeq ($(xlen),32)
	CROSS_COMPILE  := riscv32-unknown-linux-gnu-
else
	CROSS_COMPILE  := riscv64-unknown-linux-gnu-
endif

CC             := $(CROSS_COMPILE)gcc
CFLAGS         += --std=gnu99 -fno-pic -ffreestanding -fno-stack-protector -fno-builtin -Wall -mcmodel=medany -gdwarf-2

ifeq ($(xlen),32)
	CFLAGS         +=  -march=rv32{i,e,g} -D RISCV32
else
	CFLAGS         +=  -march=rv64{i,e,g} -D RISCV64
endif

# CFLAGS         += --std=gnu99 -$(ENDIAN) -G 0 -mno-abicalls -fno-pic -ffreestanding -fno-stack-protector -fno-builtin -Wa,-xgot -Wall -mxgot -mfp32 -march=r3000
LD             := $(CROSS_COMPILE)ld
# LDFLAGS        += -$(ENDIAN) -G 0 -static -n -nostdlib --fatal-warnings
LDFLAGS        += -$(ENDIAN) -G 0 -static -n -nostdlib --fatal-warnings -m elf64lriscv --no-warn-rwx-segments

HOST_CC        := cc
HOST_CFLAGS    += --std=gnu99 -O2 -Wall
HOST_ENDIAN    := $(shell lscpu | grep -iq 'little endian' && echo EL || echo EB)

ifneq ($(HOST_ENDIAN), $(ENDIAN))
# CONFIG_REVERSE_ENDIAN is checked in tools/fsformat.c (lab5)
HOST_CFLAGS    += -DCONFIG_REVERSE_ENDIAN
endif
