#include<iostream>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>



#include "pcie_uio/generic.h"
#include "pcie_uio/pci.h"
#include "pcie_uio/udmabuf.h"

#include "util.h"
#include "reg.h"
#include "addr.h"
#include "proto.h"
#include "init.h"
#include "queue.h"

int main(){
    DevPci dp;
    uint32_t buf32;
    dp.Init();

    Udmabuf physmem(0);
    std::cerr << "bar0 udmabuf size is :" << std::hex << physmem.GetSize() <<std::endl;
    
    //確保したudmaバッファを0初期化
    memset(physmem.GetVirtPtr<void>(), 0, physmem.GetSize());
    printf("Page BASE Phys = %016lx, Virt = %016lx\n",
            physmem.GetPhysPtr(),
            (uintptr_t)physmem.GetVirtPtr<void>());
    //PCIe CSREGの初期化
    initialize_pci(dp);
    //Bar0メモリマップ
    void *regspace;
    {
        int fd = open("/sys/class/uio/uio0/device/resource0", O_RDWR);
        if(fd < 0) {
            perror("open");
            return -1;
        }
        regspace = mmap(NULL, 1<<18, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(regspace == MAP_FAILED) {
            perror("mmap");
            return -1;
        }
        Reg::base_addr = regspace;
  printf("base_addr:%p\n",Reg::base_addr);
        close(fd);
    }

    initialize_hardware(regspace);
    puts("MAC Address");
    RegRa::show(0);

    const int rdescnum = 1 * 8; // must be multiple of 8
    const int rbufsz = 2 * 1024; // must be multiple of 1024
    const phys_addr tx_phys = (physmem.GetPhysPtr() + 4096 )   ;
    void * tx_virt = ((char *)physmem.GetVirtPtr<void>() + 4096);
    printf("tx_pyhs: %p\n",tx_phys);
    printf("tx_pyhs: %p\n",tx_virt);

    ReceiveQueue* rxqueue  = initialize_receive(regspace, physmem.GetPhysPtr(), physmem.GetVirtPtr<void>(), rdescnum, rbufsz);
    TransmitQueue* txqueue = initialize_transmit(regspace,tx_phys , tx_virt, rdescnum, rbufsz);

    // WriteReg(regspace, RegRdt::Offset(0), (uint32_t)rdescnum-1);

    // enable **all** interrupts
    puts("enable interrupts");
    buf32 = 0x7FFFFFFF;
    WriteReg(regspace, RegEims::kOffset, buf32);

    RegCtrl::show();
    RegStatus::show();
    RegEec::show();
    RegEemngctl::show();
    RegFcttvn::show();
    RegFcrtl::show();
    RegFcrth::show();
    RegLinks::show();
    RegRa::show(0);

    return 0;
}
