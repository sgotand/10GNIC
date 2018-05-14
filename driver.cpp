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

void bandwidth(size_t *readsize);

int main(void) {
    DevPci dp;
    uint32_t buf32;

    dp.Init();

    Udmabuf physmem(0);
    memset(physmem.GetVirtPtr<void>(), 0, 1 * 1024 * 1024);

    printf("Page BASE Phys = %016lx, Virt = %016lx\n",
            physmem.GetPhysPtr(),
            (uintptr_t)physmem.GetVirtPtr<void>());

    initialize_pci(dp);

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
        close(fd);
    }

    initialize_hardware(regspace);

    puts("Receive Addresses (only 8)");
    for(int i=0; i<8; i++) {
        uint64_t raw;
        ReadReg(regspace, RegRa::Offset(i), raw);

        HwAddr ad(ntohll(raw) >> 16);

        printf("% 3d: %d %s\n", i, ad.Valid(), ad.FormatAddr().c_str());
    }


    buf32  = 0;
    buf32 |= RegFctrl::kFlagMulticastEnable;
    buf32 |= RegFctrl::kFlagUnicastEnable;
    buf32 |= RegFctrl::kFlagBroadcastEnable;
    WriteReg(regspace, RegFctrl::kOffset, buf32);


    const int rdescnum = 1 * 8; // must be multiple of 8
    const int rbufsz = 2 * 1024; // must be multiple of 1024
    void *free_top = initialize_receive_queue(regspace, physmem, rdescnum, rbufsz);

    WriteReg(regspace, RegRdt::Offset(0), (uint32_t)rdescnum-1);

    // enable **all** interrupts
    puts("enable interrupts");
    buf32 = 0x7FFFFFFF;
    WriteReg(regspace, RegEims::kOffset, buf32);
    sleep(1);

    // FIXME ReceiveQueueをクラスにするかなんかどうにかして、こことinitialize_receive_queueの中の計算をまとめたい。
    uint8_t *rbuf0 = (uint8_t*)(((size_t)physmem.GetVirtPtr<uint8_t>() + rdescnum * 16 + 1024 - 1) & ~(1024-1));

    size_t received = 0;
#if 1
    pthread_t thread;
    if(pthread_create(&thread, NULL, (void*(*)(void*))bandwidth, &received)) {
        puts("pthread_create error");
        return 1;
    };
#endif

    uint32_t lastHead = 0;
    while(true) {
        uint32_t head;

        ReadReg(regspace, RegRdh::Offset(0), head);

        uint32_t latest = (head-1+rdescnum) % rdescnum;

        void *desc = physmem.GetVirtPtr<void>();
        for(uint32_t i = lastHead; i != head; i++, i %= rdescnum) {
            EtherReader er(rbuf0+i*2048);
            received += ((uint64_t*)desc)[i*2+1] & 0xFFFF;
        }
        lastHead = head;

#if 1
        printf("HEAD %08x\n", head);

        ReadReg(regspace, RegLinks::kOffset, buf32);
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
        usleep(1000000);
#endif

        WriteReg(regspace, RegRdt::Offset(0), latest);

    }

    return 0;
}

void bandwidth(size_t *readsize) {
    struct timespec start, end;

    while(1) {
        size_t t = *readsize;
        *readsize = 0;

        clock_gettime(CLOCK_REALTIME, &end);
        double sec = 1.0 * (end.tv_nsec - start.tv_nsec)/1000000000 + (end.tv_sec - start.tv_sec);
        start = end;

        printf("%.16f\n", sec);
        printf("received %ld\n", t);
        printf("%.8f Mbps\n", 8 * t / sec / 1000 / 1000);

        usleep(100000);
    }
}
