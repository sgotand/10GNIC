#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "pcie_uio/generic.h"
#include "pcie_uio/pci.h"
#include "pcie_uio/udmabuf.h"
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>



#include "util.h"
#include "reg.h"
#include "addr.h"
#include "proto.h"
#include "debug.h"

typedef struct ether_arp *EAP;
void bandwidth(size_t *readsize);

void debug(void* addr) {
    uint32_t buf32;
    for(unsigned int i=0; i<sizeof(info)/sizeof(Info); i++) {
        ReadReg(addr, info[i].offset, buf32);
        printf("0x%08x: % 20s %08X\n", info[i].offset, info[i].str, buf32);
    }
}

int main(void) {
    DevPci dp;
    uint16_t buf16;
    uint32_t buf32;

    dp.Init();

    Udmabuf mem(0);
    memset(mem.GetVirtPtr<void>(), 0, 1 * 1024 * 1024);

    // enable  Mastering
    dp.ReadPciReg(dp.kCommandReg, buf16);
    buf16 |= dp.kCommandRegBusMasterEnableFlag;
    dp.WritePciReg(dp.kCommandReg, buf16);

    // BAR0
    buf32 = 0x0000000c;
    dp.WritePciReg(dp.kBaseAddressReg0, buf32);

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

#if 1
    void initialize_hardware(void *addr);
    initialize_hardware(addr);
#endif

    puts("Receive Addresses (only 8)");
    for(int i=0; i<8; i++) {
        uint64_t raw;
        ReadReg(addr, RegRa::Offset(i), raw);

        HwAddr ad(ntohll(raw) >> 16);

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
        printf("RDesc[%d](%p) = %p\n", i, (void*)desc, (void*)buf);
    }

    buf32 = rbufsz / 1024;
    WriteReg(addr, RegSrrctl::Offset(0), buf32);
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






   //const int descnum = 1 * 8; // 8 entries, must be multiple of 8
   WriteReg(addr, RegTdba::Offset(0),mem.GetPhysPtr() +4096 );
   WriteReg(addr, RegTdlen::Offset(0), (uint32_t)descnum * 16);
   WriteReg(addr, RegTdh::Offset(0), (uint32_t)0);
   WriteReg(addr, RegTdt::Offset(0), (uint32_t)0);


  //同じデータを送り続ける。
  //const int rbufsz = 2 * 1024;
  const int tbufsz = 256;
 // printf("phys %16lx\n",mem.GetPhysPtr());
 // printf("virt %p\n",mem.GetVirtPtr<uint8_t>());

  size_t tbufbase = (mem.GetPhysPtr() +4096 + descnum * 16 + 2047) / 2048 * 2048;
  uint64_t  databuf = ((uint64_t) mem.GetVirtPtr<uint8_t>() + 4096 + descnum * 16 + 2047 ) /2048 * 2048;
  printf("tbufbase %p\n",(void*)tbufbase);
  printf("databuf %p\n",(void*)databuf);
  for(int i=0; i<descnum; i++) {
    uint64_t *desc = &mem.GetVirtPtr<uint64_t>()[i * 2];
    size_t buf = tbufbase ; //+ i * rbufsz;  
    desc[0] = buf;
    desc[1] |= 1<<24; //set End of packet 
    desc[1] |= 42;
    printf("RDesc[%d](%p) = %p\n", i,(void*)desc, (void*)buf);
    printf("RDesc[%d](%p) = %llx\n",i,(void *) &desc[1] ,desc[1]);
  }








    // enable **all** interrupts
    puts("enable interrupts");
    buf32 = 0x7FFFFFFF;
    WriteReg(addr, RegEims::kOffset, buf32);
    sleep(1);

    uint8_t *rbuf0 = (uint8_t*)(((size_t)mem.GetVirtPtr<uint8_t>() + descnum * 16 + 2047) / 2048 * 2048);

    size_t received = 0;
#if 0
    pthread_t thread;
    if(pthread_create(&thread, NULL, (void*(*)(void*))bandwidth, &received)) {
        puts("pthread_create error");
        return 1;
    };
#endif

    uint32_t lastHead = 0;
    while(true) {
    
        uint32_t head;

        ReadReg(addr, RegRdh::Offset(0), head);
        uint32_t latest = (head-1+descnum) % descnum;

        void *desc = mem.GetVirtPtr<void>();
        for(uint32_t i = lastHead; i != head; i++, i %= descnum) {
            EtherReader er(rbuf0+i*2048);
            received += ((uint64_t*)desc)[i*2+1] & 0xFFFF;
            

	}
        lastHead = head;
  struct ether_header *ehp;
 ehp =(struct ether_header *) (rbuf0+latest*2048);
	
if( ntohs(ehp->ether_type) == ETHERTYPE_ARP ){


  struct ether_arp *eap ;
  eap =(struct ether_arp *) (rbuf0+latest*2048 + 14);


  printf("arp_hrd:%-4d\n",ntohs(eap->arp_hrd));
  printf("arp_pro:%04x\n",ntohs(eap->arp_pro));
  printf("arp_hln:%-4d\n",eap->arp_hln);
  printf("arp_pln:%-4d\n",eap->arp_pln);
  printf("arp_op :%-4d\n",ntohs(eap->arp_op));


  void *p = malloc(42);
  struct ether_header *resp_ehp =(struct ether_header *) p;
  struct ether_arp *resp_eap =(struct ether_arp *) (p + 14);
  //memcopy(resp_ehp->ether_dhost,eap->arp_sha,6);
  //memcopy(resp_ehp->ether_shost,        ,6);//自分RegRa
  resp_ehp->ether_type=htons(ETHERTYPE_ARP);

  resp_eap->arp_hrd = eap->arp_hrd;
  resp_eap->arp_pro = eap->arp_pro;
  resp_eap->arp_hln = eap->arp_hln;
  resp_eap->arp_pln = eap->arp_pln;
  resp_eap->arp_op  =htons(ARPOP_REPLY);
  //memcopy(resp_eap->arp_sha,ether_aton(RegRa::get(0)),6);//自分RegRa
  
  resp_ehp->ether_shost[0] =0xa0;
  resp_ehp->ether_shost[1] =0x36;
  resp_ehp->ether_shost[2] =0x9f;
  resp_ehp->ether_shost[3] =0xf2;
  resp_ehp->ether_shost[4] =0xe4;
  resp_ehp->ether_shost[5] =0x18;
  resp_eap->arp_sha[0] =0xa0;  
  resp_eap->arp_sha[1] =0x36;  
  resp_eap->arp_sha[2] =0x9f;  
  resp_eap->arp_sha[3] =0xf2;  
  resp_eap->arp_sha[4] =0xe4;  
  resp_eap->arp_sha[5] =0x18;

  //memcopy(resp_eap->arp_spa,inet_aton("1.2.3.5"), 4);//1.2.3.5
  resp_eap->arp_spa[0] = 1;
  resp_eap->arp_spa[1] = 2;
  resp_eap->arp_spa[2] = 3;
  resp_eap->arp_spa[3] = 5;
  for(int i = 0; i < 6;i++){
  	resp_eap->arp_tha[i] = ehp->ether_shost[i];
	resp_eap->arp_sha[i] = ehp->ether_shost[i];
	resp_ehp->ether_dhost[i] = eap->arp_sha[i];
	if(i<5)resp_eap->arp_tpa[i]=eap->arp_spa[i];
  }
  //memcpy(resp_eap->arp_tha,ehp->ether_shost,6);
  //memcpy(resp_eap->arp_tpa,eap->arp_spa ,   4);
  for(int j=0; j<42; j++)
      printf("%02x ", ((uint8_t *)p)[j]);
  puts("");

  memcpy((void *)databuf,p,42);

   WriteReg(addr, RegTdt::Offset(0), (uint32_t)1);
}








#if 1
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
        usleep(1000000);
#endif

        WriteReg(addr, RegRdt::Offset(0), latest);

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
