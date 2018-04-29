#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>

#include "pcie_uio/generic.h"
#include "pcie_uio/pci.h"
#include "pcie_uio/mem.h"

#include "reg.h"
#include "addr.h"
#include "debug.h"

void debug() {
}
int main(void) {
	DevPci dp;
	dp.Init();

	Memory mem(2 * 1024 * 1024);
	memset(mem.GetVirtPtr<void>(), 0, 2 * 1024 * 1024);

	int fd = open("/sys/class/uio/uio0/device/resource0", O_RDWR);
	if(fd < 0) {
		perror("open");
		return -1;
	}
	void *addr = mmap(NULL, 1<<18, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(addr == MAP_FAILED) {
		perror("mmap");
		return -1;
	}

	close(fd);

	uint16_t buf16;
	uint32_t buf32;
	uint64_t buf64;

	// CommandReg
	dp.ReadPciReg(dp.kCommandReg, buf16);
	buf16 |= dp.kCommandRegBusMasterEnableFlag;
	dp.WritePciReg(dp.kCommandReg, buf16);

	// BAR0
	buf32 = 0x0000000c;
	dp.WritePciReg(dp.kBaseAddressReg0, buf32);
	dp.ReadPciReg(dp.kBaseAddressReg0, buf32);
	printf("BAR0 %08x\n", buf32);

	// disable interrupts
	puts("disable interrupts");
	buf32 = 0x7FFFFFFF;
	WriteReg(addr, RegEimc::kOffset, buf32);
	sleep(1);

	// read Status Reg
	ReadReg(addr, RegStatus::kOffset, buf32);
	printf("Status: %08x\n", buf32);
	
	// device reset
	puts("device reset");
	ReadReg(addr, RegCtrl::kOffset, buf32);
	printf("CTRL: %08x\n", buf32);
	buf32 |= RegCtrl::kFlagDeviceReset;
	WriteReg(addr, RegCtrl::kOffset, buf32);
	sleep(1);

	// disable interrupt
	puts("disable interrupt");
	buf32 = 0x7FFFFFFF;
	WriteReg(addr, RegEimc::kOffset, buf32);
	sleep(1);

	// setting
	for(int i=0x3200; i<=0x32a0; i+=0x4) {
		((uint32_t*)addr)[i/4] = 0;
	}
	((uint32_t*)addr)[0x3d00/4] = 0;
	for(int i=0x3260; i<0x32a0; i+=0x4) {
		((uint32_t*)addr)[i/4] = 1<<10;
	}
	
	// link reset
	puts("link reset");
	ReadReg(addr, RegCtrl::kOffset, buf32);
	buf32 |= RegCtrl::kFlagLinkReset;
	WriteReg(addr, RegCtrl::kOffset, buf32);
	sleep(1);


	// read EEMNGCTL
	for(int i=0; i<3; i++) {
		uint32_t a;
		a = ((uint32_t*)addr)[0x0000/4];
		printf("CTRL: %08x\n", a);
		a = ((uint32_t*)addr)[0x10110/4];
		printf("EEMNGCTL: %08x\n", (a));
		a = ((uint32_t*)addr)[0x2f00/4];
		printf("RDRXCTL: %08x\n", (a));
		sleep(1);

	}

	puts("Receive Addresses");
	for(int i=0; i<8; i++) {
		uint64_t raw;
		ReadReg(addr, RegRa::Offset(i), raw);

		Addr ad(raw);

		printf("% 3d: %d %s\n", i, ad.Valid(), ad.FormatAddr().c_str());
	}

	WriteReg(addr, RegFctrl::kOffset, (uint32_t)(RegFctrl::kFlagMulticastEnable | RegFctrl::kFlagUnicastEnable | RegFctrl::kFlagBroadcastEnable));


	printf("Page BASE Phys = %016lx, Virt = %p\n", mem.GetPhysPtr(), mem.GetVirtPtr<void>());

	const int descnum = 1 * 8; // 8 entries, must be multiple of 8
	WriteReg(addr, RegRdba::Offset(0), mem.GetPhysPtr());
	WriteReg(addr, RegRdlen::Offset(0), (uint32_t)descnum * 16);
	WriteReg(addr, RegRdh::Offset(0), (uint32_t)0);
	WriteReg(addr, RegRdt::Offset(0), (uint32_t)0);


	const int rbufsz = 2 * 1024;
	size_t rbufbase = (mem.GetPhysPtr() + descnum * 16 + 2047) / 2048 * 2048;
	for(int i=0; i<descnum; i++) {
		uint64_t *desc = &mem.GetVirtPtr<uint64_t>()[i * 2];
		size_t buf = rbufbase + i * rbufsz;
		desc[0] = buf;
		printf("RDesc[%d](%p) = %p\n", i, desc, (void*)buf);
	}

	buf32 = rbufsz / 1024;
	WriteReg(addr, RegSrrctl::Offset(0), (uint32_t)0x2);
	WriteReg(addr, RegRscctl::Offset(0), (uint32_t)0);
	WriteReg(addr, RegRxdctl::Offset(0), RegRxdctl::kFlagReceiveQueueEnable);

	while(true) {
		ReadReg(addr, RegRxdctl::Offset(0), buf32);
		if(buf32 & RegRxdctl::kFlagReceiveQueueEnable)
			break;
		__asm__ volatile("" ::: "memory");
	}
	WriteReg(addr, RegRdt::Offset(0), (uint32_t)descnum-1);
	WriteReg(addr, RegRxctrl::kOffset, RegRxctrl::kFlagEnable);

	// enable **all** interrupts
	puts("enable interrupts");
	buf32 = 0x7FFFFFFF;
	WriteReg(addr, RegEims::kOffset, buf32);
	sleep(1);

	uint8_t *rbuf0 = (uint8_t*)(((size_t)mem.GetVirtPtr<uint8_t>() + descnum * 16 + 2047) / 2048 * 2048);
	printf("rbuf0 %p\n", rbuf0);
	for(int i=0; i<sizeof(info)/sizeof(Info); i++) {
		ReadReg(addr, info[i].offset, buf32);
		printf("0x%08x: % 20s %08X\n", info[i].offset, info[i].str, buf32);
	}
	while(true) {
		ReadReg(addr, RegRdh::Offset(0), buf32);
		if(buf32 != 0) break;
		sleep(1);
	}
	while(true) {
		ReadReg(addr, RegRdh::Offset(0), buf32);
		printf("HEAD %08x\n", buf32);
		ReadReg(addr, RegStatus::kOffset, buf32);
		printf("Status %08x\n", buf32);

		uint64_t *desc = &mem.GetVirtPtr<uint64_t>()[0];
		printf("%016lx %016lx\n", desc[0], desc[1]);
		for(int i=0; i<8; i++){
			for(int j=0; j<32; j++)
				printf("%02x ", rbuf0[i*2048 + j]);
			puts("");
		}
		ReadReg(addr, RegLinks::kOffset, buf32);
		const char *link_speed[] = {"None", "100M", "1G", "10G"};
		printf("LINKS: %s(%s)\n", (buf32 & RegLinks::kFlagLinkStatusUp) ? "UP" : "DOWN", link_speed[(buf32 & RegLinks::kMaskLinkSpeed) >> RegLinks::kOffsetLinkSpeed]);
		sleep(1);
	}


	return 0;
}
