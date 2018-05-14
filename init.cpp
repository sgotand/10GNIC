#include <stdint.h>
#include <unistd.h>

#include "util.h"
#include "reg.h"
#include "addr.h"
#include "proto.h"

void initialize_hardware(void *addr) {
	uint32_t buf32;

	//Initialization Sequence
	puts("Initialize");

	// disable interrupts
	buf32 = 0x7FFFFFFF; //set bits [30:0] , bits 31 is reserved
	WriteReg(addr, RegEimc::kOffset, buf32);

	// read Status Reg
	ReadReg(addr, RegStatus::kOffset, buf32);
	printf("Status: %08x\n", buf32);

	// global reset (see 4.6.3.2)
	ReadReg(addr, RegCtrl::kOffset, buf32);
	buf32 |= RegCtrl::kFlagDeviceReset;
	WriteReg(addr, RegCtrl::kOffset, buf32);
	while(true) {
		ReadReg(addr, RegCtrl::kOffset,buf32);
		if(!(buf32 & RegCtrl::kFlagDeviceReset)) break;
		usleep(1000);
	};
	usleep(1000);

	// setting flow control (as is not enabled)
	// FIXME hoge
	for(int i=0x3200; i<=0x32a0; i+=0x4) {
		((uint32_t*)addr)[i/4] = 0;
	}
	((uint32_t*)addr)[0x3d00/4] = 0;
	for(int i=0x3260; i<0x32a0; i+=0x4) {
		((uint32_t*)addr)[i/4] = 1<<10;
	}

	// link reset
	ReadReg(addr, RegCtrl::kOffset, buf32);
	buf32 |= RegCtrl::kFlagLinkReset;
	WriteReg(addr, RegCtrl::kOffset, buf32);

	// disable interrupt
	buf32 = 0x7FFFFFFF;
	WriteReg(addr, RegEimc::kOffset, buf32);


	// Wait for the NVM auto-read completion.
	while(true) {
		ReadReg(addr,RegEec::kOffset,buf32);
		if(buf32 & RegEec::kFlagAutoRd) break;
		usleep(1000); //for mitigating busy wait
	}

	usleep(1000);

	while(1){
		ReadReg(addr,RegRdrxctl::kOffset,buf32);
		if(buf32 & RegRdrxctl::kFlagDmaidone) break;
		usleep(1000); //for mitigating busy wait
	}


	{
		// read EEMNGCTL (Manageability EEPROM Mode Control Register)
		buf32 = ((uint32_t*)addr)[0x0000/4];
		printf("CTRL: %08x\n", buf32);
		buf32 = ((uint32_t*)addr)[0x10110/4];
		printf("EEMNGCTL: %08x\n", buf32);
		buf32 = ((uint32_t*)addr)[0x2f00/4];
		printf("RDRXCTL: %08x\n", buf32);
		sleep(1);
	}
	return;
}
