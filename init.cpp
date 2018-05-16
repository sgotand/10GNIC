#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include "pcie_uio/udmabuf.h"
#include "pcie_uio/pci.h"

#include "util.h"
#include "reg.h"
#include "addr.h"
#include "proto.h"
#include "queue.h"

void initialize_pci(DevPci &dp) {
    uint32_t buf32;
    uint16_t buf16;

    // enable  Mastering
    dp.ReadPciReg(dp.kCommandReg, buf16);
    buf16 |= dp.kCommandRegBusMasterEnableFlag;
    dp.WritePciReg(dp.kCommandReg, buf16);

    // BAR0
    buf32 = 0x0000000c;
    dp.WritePciReg(dp.kBaseAddressReg0, buf32);
}

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

ReceiveQueue* initialize_receive(void *regspace, phys_addr physbase, void *virtbase, size_t descnum, size_t bufsz) {
    uint32_t buf32;
    assert((physbase & (1024ll-1ll)) == 0);
    assert(descnum % 8 == 0);
    assert(bufsz % 1024 == 0);

    buf32  = 0;
    buf32 |= RegFctrl::kFlagMulticastEnable;
    buf32 |= RegFctrl::kFlagUnicastEnable;
    buf32 |= RegFctrl::kFlagBroadcastEnable;
    WriteReg(regspace, RegFctrl::kOffset, buf32);

    WriteReg(regspace, RegRdba::Offset(0), physbase);
    WriteReg(regspace, RegRdlen::Offset(0), (uint32_t)descnum * 16);
    WriteReg(regspace, RegRdh::Offset(0), (uint32_t)0);
    WriteReg(regspace, RegRdt::Offset(0), (uint32_t)0);

    ReceiveQueue *rq = new ReceiveQueue(descnum, bufsz, physbase, virtbase);

    buf32 = bufsz / 1024;
    WriteReg(regspace, RegSrrctl::Offset(0), buf32);
    WriteReg(regspace, RegRscctl::Offset(0), (uint32_t)0);
    WriteReg(regspace, RegRxdctl::Offset(0), RegRxdctl::kFlagReceiveQueueEnable);

    while(true) {
        ReadReg(regspace, RegRxdctl::Offset(0), buf32);
        if(buf32 & RegRxdctl::kFlagReceiveQueueEnable)
            break;
        __asm__ volatile("" ::: "memory");
    }
    WriteReg(regspace, RegRxctrl::kOffset, RegRxctrl::kFlagEnable);

    return rq;
}

TransmitQueue* initialize_transmit(void *regspace, phys_addr physbase, void *virtbase, size_t descnum , size_t bufsz) {
    uint32_t buf32;

    assert((physbase & (1024-1)) == 0);
    assert(descnum % 8 == 0);

    ReadReg(regspace, RegDmatxctl::kOffset, buf32);
    assert(!(buf32 & RegDmatxctl::kFlagTransmitEnable));
    buf32 &= ~RegDmatxctl::kFlagTransmitEnable;
    WriteReg(regspace, RegDmatxctl::kOffset, buf32);

    WriteReg(regspace, RegTdba::Offset(0), physbase);
    WriteReg(regspace, RegTdlen::Offset(0), (uint32_t)descnum * 16);
    WriteReg(regspace, RegTdh::Offset(0), (uint32_t)0);
    WriteReg(regspace, RegTdt::Offset(0), (uint32_t)0);

    TransmitQueue *tq = new TransmitQueue(descnum, physbase, virtbase);

    ReadReg(regspace, RegDmatxctl::kOffset, buf32);
    buf32 |= RegDmatxctl::kFlagTransmitEnable;
    WriteReg(regspace, RegDmatxctl::kOffset, buf32);

    return tq;
}
