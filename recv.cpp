#include<assert.h>
#include<stdint.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>
#include<sys/time.h>

#include "pcie_uio/generic.h"
#include "pcie_uio/pci.h"
#include "pcie_uio/mem.h"

#include "util.h"
#include "reg.h"
#include "addr.h"
#include "proto.h"
#include "debug.h"

void debug(void* addr) {
	uint32_t buf32;
	for(int i=0; i<sizeof(info)/sizeof(Info); i++) {
		ReadReg(addr, info[i].offset, buf32);
		printf("0x%08x: % 20s %08X\n", info[i].offset, info[i].str, buf32);
	}
}
int main(void) {
	// pcie_uio/pci.hで定義
	DevPci dp;
	uint16_t buf16;
	uint32_t buf32;

	// _uiofd,_configfdをオープン
	dp.Init();

// PCIe Configuraiton
	// enable  Mastering
	dp.ReadPciReg(dp.kCommandReg, buf16);
	buf16 |= dp.kCommandRegBusMasterEnableFlag;
	dp.WritePciReg(dp.kCommandReg, buf16);

	// BAR0
	buf32 = 0x0000000c;
	dp.WritePciReg(dp.kBaseAddressReg0, buf32);


//map dgeneral control registers
	// pcie_uio/mem.hで定義
	Memory mem(2 * 1024 * 1024);
	memset(mem.GetVirtPtr<void>(), 0, 2 * 1024 * 1024);

	//map bar0 registers
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


//Initialization Sequence
	puts("\n~~~~Initialization sequence~~~~\n");

	// disable interrupts bywriting to teh EIMC register
	puts("\n1. Disable interrupts");
	buf32 = 0x7FFFFFFF; //set bits [30:0] , bits 31 is reserved
	WriteReg(addr, RegEimc::kOffset, buf32);

	// read Status Reg (for print , unnecessary)
	ReadReg(addr, RegStatus::kOffset, buf32);
	printf("Status: %08x\n", buf32);


	// global reset (see 4.6.3.2)
	/*
	initialization sequence の該当項目には
		"Global reset = software reset + link reset"
	と記述あり。
	CTRL.RST(Device reset) には
		"also refferd to as a software reset or global reset"
	と記述あり???
	link reset(set CTRL.LRST)???
	*/
	puts("\n2.1. device reset () software");
	ReadReg(addr, RegCtrl::kOffset, buf32);
	buf32 |= RegCtrl::kFlagDeviceReset;
	WriteReg(addr, RegCtrl::kOffset, buf32);
	//poll the CTRL.RST until it is cleared
	while(1) {
		ReadReg(addr, RegCtrl::kOffset,buf32);
		if(!(buf32 & RegCtrl::kFlagDeviceReset)) break;
		usleep(1000); //for mitigating busy wait
	};
	//need  waiting at least 10ms after polling;
	usleep(1100); // wait 10% extra

	// setting flow control (as is not enabled)
	puts("\n2.2 setting flow control");
	for(int i=0x3200; i<=0x32a0; i+=0x4) {
		((uint32_t*)addr)[i/4] = 0;
	}
	((uint32_t*)addr)[0x3d00/4] = 0;
	for(int i=0x3260; i<0x32a0; i+=0x4) {
		((uint32_t*)addr)[i/4] = 1<<10;
	}

	// link reset
	puts("\n2.3. link reset");
	ReadReg(addr, RegCtrl::kOffset, buf32);
	buf32 |= RegCtrl::kFlagLinkReset;
	WriteReg(addr, RegCtrl::kOffset, buf32);

	// disable interrupt (see 4.6.3.1)
	puts("\ndisable interrupt (after issuing a global reset)");
	buf32 = 0x7FFFFFFF; //set bits [30:0] , bits 31 is reserved
	WriteReg(addr, RegEimc::kOffset, buf32);

	// Wait for the NVM auto-read completion.
	puts("\n\n3. Wait for the NVM auto-read completion.");
	while(1){
		ReadReg(addr,RegEec::kOffset,buf32);
		if(buf32 & RegEec::kFlagAutoRd) break;
		usleep(1000); //for mitigating busy wait
	}

	puts("\n\n4. Wait for manegeability configuration done indication");
	puts("\tignore this stage(but wait 1 msec)");
	usleep(1000);



	puts("\n\n5. Wait until DMA initialization complets");
	while(1){
		ReadReg(addr,RegRdrxctl::kOffset,buf32);
		if(buf32 & RegRdrxctl::kFlagDmaidone) break;
		usleep(1000); //for mitigating busy wait
	}


	puts("\n\n6. Setup the PHY and the link");
	puts("\tdo nothing in this  stage");

	puts("\n\n7. Initialize  all statistical counters");
	puts("\tdo nothing in this  stage");



	// read EEMNGCTL (Manageability EEPROM Mode Control Register)
	{
		// maybe no need
		buf32 = ((uint32_t*)addr)[0x0000/4];
		printf("CTRL: %08x\n", buf32);
		buf32 = ((uint32_t*)addr)[0x10110/4];
		printf("EEMNGCTL: %08x\n", buf32);
		buf32 = ((uint32_t*)addr)[0x2f00/4];
		printf("RDRXCTL: %08x\n", buf32);
		sleep(1);
	}

	puts("Receive Addresses (only 8)");
	for(int i=0; i<8; i++) {
		uint64_t raw;
		ReadReg(addr, RegRa::Offset(i), raw);

		HwAddr ad(ntohll(raw));

		printf("% 3d: %d %s\n", i, ad.Valid(), ad.FormatAddr().c_str());
	}


	WriteReg(addr, RegFctrl::kOffset, (uint32_t)(RegFctrl::kFlagMulticastEnable | RegFctrl::kFlagUnicastEnable | RegFctrl::kFlagBroadcastEnable));
	printf("Page BASE Phys = %016lx, Virt = %p\n", mem.GetPhysPtr(), mem.GetVirtPtr<void>());

	const int descnum = 500 * 8; // 8 entries, must be multiple of 8
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
	debug(addr);

	uint32_t lastHead = 0;

	struct timespec start, end;
	while(true) {
		uint32_t head;

		ReadReg(addr, RegRdh::Offset(0), head);
		clock_gettime(CLOCK_REALTIME, &end);
		double sec = 1.0 * (end.tv_nsec - start.tv_nsec)/1000000000 + (end.tv_sec - start.tv_sec);
		printf("%.16f\n", sec);
		start = end;
		//if(head == lastHead) continue;

		uint32_t latest = (head-1+descnum) % descnum;

		uint32_t received = 0;
		for(uint32_t i = lastHead; i != head; i++, i %= descnum) {
			void *desc = mem.GetVirtPtr<void>();
			received += ((uint64_t*)desc)[i*2+1] & 0xFFFF;
		}
		printf("received %d\n", received);
		printf("Gbps %.8f\n", 8 * received / sec / 1024 / 1024);

		lastHead = head;

		printf("HEAD %08x\n", head);
#if 0

		printf("HEAD %08x\n", head);

		ReadReg(addr, RegLinks::kOffset, buf32);
		const char *link_speed[] = {"None", "100M", "1G", "10G"};
		printf("LINKS: %s(%s)\n", (buf32 & RegLinks::kFlagLinkStatusUp) ? "UP" : "DOWN", link_speed[(buf32 & RegLinks::kMaskLinkSpeed) >> RegLinks::kOffsetLinkSpeed]);

		for(int i=0; i<8; i++){
			printf("%s ", i == latest ? "=>" : "  ");
			for(int j=0; j<32; j++)
				printf("%02x ", rbuf0[i*2048 + j]);
			puts("");
		}
		puts("");

		{
			EtherReader er(rbuf0+latest*2048);
			printf("Latest Packet Detail:\n");

			puts("Ether");
			printf(" Src: %s\n", er.Src().FormatAddr().c_str());
			printf(" Dst: %s\n", er.Dst().FormatAddr().c_str());
			printf("Type: %s\n", EtherReader::Format(er.Type()).c_str());

			switch(er.Type()) {
				case EtherReader::ARP:
				       	{
						ArpReader ar(er.Data());
						puts(" ARP");
						printf("  Src: %s %s\n",
							       	ar.HwSrc().FormatAddr().c_str(),
							       	ar.IpSrc().Format().c_str()
								);
						printf("  Dst: %s %s\n",
							       	ar.HwDst().FormatAddr().c_str(),
							       	ar.IpDst().Format().c_str()
								);
					}
			}
		}
		puts("===============================================================================");
#endif

		WriteReg(addr, RegRdt::Offset(0), latest);

		usleep(10000);
	}


	return 0;
}
