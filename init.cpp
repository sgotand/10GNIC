#include<stdio.h>
#include<stdint.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>

#include "pcie_uio/generic.h"
#include "pcie_uio/pci.h"

#include "reg.h"

int main(void) {
	DevPci dp;
	dp.Init();

	int fd = open("/sys/class/uio/uio0/device/resource0", O_RDWR);
	if(fd < 0) {
		perror("open");
		return -1;
	}
	void *addr = mmap(NULL, 1<<18, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(addr < 0) {
		perror("mmap");
		return -1;
	}

	close(fd);

	uint16_t buf16;
	uint32_t buf32;

	// CommandReg
	dp.ReadPciReg(dp.kCommandReg, buf16);
	buf16 |= db.kCommandRegBusMasterEnableFlag;
	dp.WritePciReg(dp.kCommandReg, buf16);

	// BAR0
	buf32 = 0x000c;
	dp.WritePciReg(dp.kBaseAddressReg0, buf32);
	dp.ReadPciReg(dp.kBaseAddressReg0, buf32);
	printf("%08x\n", buf32);

	// disable interrupt
	printf("write EIMC\n");
	buf32 = 0x7FFFFFFF;
	WriteReg(addr, kOffsetEIMC, buf32);
	sleep(1);

	// read Status Reg
	ReadReg(addr, kOffsetSTATUS, a);
	printf("Status Reg: %08x\n", a);
	
	// device reset
	ReadReg(addr, kOffsetCTRL, buf32);
	printf("CTRL: %08x\n", a);
	((uint32_t*)addr)[0x0000/4] = a | (1<<26);
	sleep(1);

	// disable interrupt
	printf("write EIMC\n");
	WriteReg(addr,kOffsetEIMC,(uit32_t)0x7FFFFFFF);
	//((uint32_t*)addr)[0x0888/4] = 0x7FFFFFFF;
	sleep(1);

	// setting
	for(int i=0x3200; i<=0x32a0; i+=0x4) {
		((uint32_t*)addr)[i/4] = 0;
	}
	((uint32_t*)addr)[0x3d00/4] = 0;
	for(int i=0x3260; i<0x32a0; i+=0x4) {
		//((uint32_t*)addr)[i/4] = 1<<8;
	}
	
	((uint32_t*)addr)[0x0000/4] = a | (1<<3);

	// read 10sec EEMNGCTL
	for(int i=0; i<1000; i++) {
			a = ((uint32_t*)addr)[0x0000/4];
			printf("CTRL: %08x\n", a);
			a = ((uint32_t*)addr)[0x10110/4];
			printf("EEMNGCTL: %08x\n", (a));
			a = ((uint32_t*)addr)[0x2f00/4];
			printf("RDRXCTL: %08x\n", (a));
	}

	printf("write EIMS\n");
	((uint32_t*)addr)[0x0880/4] = 0x7FFFFFFF;
	sleep(1);

	return 0;
}
