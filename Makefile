TARGET_KEYWORD := 10G
TARGET_DEFAULT_DRIVER := ixgbe

DEPS := $(filter %.d, $(subst .o,.d, $(OBJS)))
CXXFLAGS := -g -std=c++14 -MMD -MP -Wall -Wpedantic -pthread -I./pcie_uio

all: driver

-include $(DEPS)
include pcie_uio/common.mk


clean:
	rm -f send recv *.o *.d

run_send: send init
	sudo ./send

run_recv: recv init
	sudo ./recv

run_driver: driver init
	sudo ./driver

send: send.o reg.o
	g++ $(CXXFLAGS) -o $@ $^

recv: init.o recv.o reg.o
	g++ $(CXXFLAGS) -o $@ $^

driver: init.o driver.o reg.o
	g++ $(CXXFLAGS) -o $@ $^

init:
	sudo rmmod udmabuf || true
	sudo insmod udmabuf/udmabuf.ko udmabuf0=1048576 udmabuf1=1048576 || true
	sudo sh -c 'echo 0 > /sys/class/udmabuf/udmabuf0/sync_offset'
	sudo sh -c 'echo 1048576 > /sys/class/udmabuf/udmabuf0/sync_size'
	sudo sh -c 'echo 0 > /sys/class/udmabuf/udmabuf0/sync_direction'
	sudo sh -c 'echo 1 > /sys/class/udmabuf/udmabuf0/sync_for_cpu'
	sudo sh -c 'echo 1 > /sys/class/udmabuf/udmabuf0/sync_for_device'
	sudo sh -c 'echo 0 > /sys/class/udmabuf/udmabuf1/sync_offset'
	sudo sh -c 'echo 1048576 > /sys/class/udmabuf/udmabuf1/sync_size'
	sudo sh -c 'echo 0 > /sys/class/udmabuf/udmabuf1/sync_direction'
	sudo sh -c 'echo 1 > /sys/class/udmabuf/udmabuf1/sync_for_cpu'
	sudo sh -c 'echo 1 > /sys/class/udmabuf/udmabuf1/sync_for_device'

.PHONY: all init
