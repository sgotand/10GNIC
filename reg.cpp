#include<cstdint>
#include<cassert>
#include<cstdio>
#include<iostream>
#include<iomanip>

#include "reg.h"
#include "addr.h"
#include "pcie_uio/generic.h"


// Read Write Functions
void ReadReg(void *addr, uint32_t off, uint8_t &val) {
    val = ((uint8_t *)addr)[off];
}
void ReadReg(void *addr, uint32_t off, uint16_t &val) {
    val = ((uint16_t *)addr)[off/2];
}
void ReadReg(void *addr, uint32_t off, uint32_t &val) {
    val = ((uint32_t *)addr)[off/4];
}
void ReadReg(void *addr, uint32_t off, uint64_t &val) {
    uint32_t t;
    ReadReg(addr, off, t);
    val  = t;
    ReadReg(addr, off+4, t);
    val |= (uint64_t)t << 32;
}

void WriteReg(void *addr, uint32_t off, uint8_t val) {
    ((uint8_t *)addr)[off] = val;
}
void WriteReg(void *addr, uint32_t off, uint16_t val) {
    ((uint16_t *)addr)[off/2] = val;
}
void WriteReg(void *addr, uint32_t off, uint32_t val) {
    ((uint32_t *)addr)[off/4] = val;
}
void WriteReg(void *addr, uint32_t off, uint64_t val) {
    WriteReg(addr, off, (uint32_t)(val & 0xFFFFFFFF));
    WriteReg(addr, off+4, (uint32_t)(val >> 32));
}




void RegCtrl::show(){
    uint32_t t;
    if(base_addr == NULL){
        std::cerr<<"base_addr is NULL"<<std::endl;
        return;
    }
    ReadReg(base_addr,kOffset,t);
    std::cerr<<"[Info]RegCtrl:"<<std::endl;
    std::cerr<<"  kOffset   ="<< std::hex<<std::setfill('0') <<kOffset<<std::endl;
    std::cerr<<"  .LRST     ="<< (IsFlagClear(t,kFlagLinkReset) ? '1' : '0') <<std::endl;
    std::cerr<<"  .RST      ="<< (IsFlagSet(t,kFlagDeviceReset) ? '1' : '0')<<std::endl;
    std::cerr<<"  AllFlags  ="<< std::hex<<std::setfill('0') <<t;
}




void RegStatus::show(){
    uint32_t t;
    if(base_addr == NULL){
        std::cerr<<"base_addr is NULL"<<std::endl;
        return;
    }
    ReadReg(base_addr,kOffset,t);
    std::cerr<<"[Info]RegStatu:"<<std::endl;
    std::cerr<<"  kOffset   ="<< std::hex<<std::setfill('0') <<kOffset<<std::endl;
    std::cerr<<"  AllFlags  ="<< std::hex<<std::setfill('0') <<t;
}



void RegEec::show(){
    uint32_t t;
    if(base_addr == NULL){
        std::cerr<<"base_addr is NULL"<<std::endl;
        return;
    }
    ReadReg(base_addr,kOffset,t);
    std::cerr<<"[Info]RegEec:"<<std::endl;
    std::cerr<<"  kOffset   ="<< std::hex<<std::setfill('0') <<kOffset<<std::endl;
    std::cerr<<"  .AUTO_RD  ="<< (IsFlagSet(t,kFlagAutoRd) ? '1' : '0')<<std::endl;
    std::cerr<<"  AllFlags  ="<< std::hex<<std::setfill('0') <<t;
}



void RegEemngctl::show(){
    uint32_t t;
    if(base_addr == NULL){
        std::cerr<<"base_addr is NULL"<<std::endl;
        return;
    }
    ReadReg(base_addr,kOffset,t);
    std::cerr<<"[Info]RegEemngctl:"<<std::endl;
    std::cerr<<"  kOffset   ="<< std::hex<<std::setfill('0') <<kOffset<<std::endl;
    std::cerr<<"  .CFG_DONE0  ="<< (IsFlagSet(t,kFlagCfgDone0) ? '1' : '0')<<std::endl;
    std::cerr<<"  .CFG_DONE1  ="<< (IsFlagSet(t,kFlagCfgDone1) ? '1' : '0')<<std::endl;
    std::cerr<<"  AllFlags  ="<< std::hex<<std::setfill('0') <<t;
}



void RegFcttvn::show(){
    // uint32_t t;
    // if(base_addr == NULL){
    // std::cerr<<"base_addr is NULL"<<std::endl;
// }
    // ReadReg(base_addr,kOffset,t);
    std::cerr<<"[Info]RegFcttvn:"<<std::endl;
    std::cerr<<"  kOffset   ="<< std::hex<<std::setfill('0') <<kOffset<<std::endl;
    std::cerr<<"  kNlimit   ="<< kNlimit  <<std::endl;
    // std::cerr<<"  AllFlags  ="<< std::hex<<std::setfill('0') <<t;
}


void RegFcrtl::show(){
    // uint32_t t;
    // if(base_addr == NULL){
    //std::cerr<<"base_addr is NULL"<<std::endl;
// }
    // ReadReg(base_addr,kOffset,t);
    std::cerr<<"[Info]RegFcrtl:"<<std::endl;
    std::cerr<<"  kOffset   ="<< std::hex<<std::setfill('0') <<kOffset<<std::endl;
    std::cerr<<"  kNlimit   ="<< kNlimit  <<std::endl;
    // std::cerr<<"  AllFlags  ="<< std::hex<<std::setfill('0') <<t;
}


void RegFcrth::show(){
    // uint32_t t;
    // if(base_addr == NULL){
    // std::cerr<<"base_addr is NULL"<<std::endl;
// }
    // ReadReg(base_addr,kOffset,t);
    std::cerr<<"[Info]RegFcrth:"<<std::endl;
    std::cerr<<"  kOffset   ="<< std::hex<<std::setfill('0') <<kOffset<<std::endl;
    std::cerr<<"  kNlimit   ="<< kNlimit  <<std::endl;
    // std::cerr<<"  AllFlags  ="<< std::hex<<std::setfill('0') <<t;
}


void RegLinks::show(){
    uint32_t t;
    if(base_addr == NULL){
        std::cerr<<"base_addr is NULL"<<std::endl;
        return;
    }
    ReadReg(base_addr,kOffset,t);
    std::cerr<<"[Info]RegLinks:"<<std::endl;
    std::cerr<<"  kOffset   ="<< std::hex<<std::setfill('0') <<kOffset<<std::endl;
    std::cerr<<"  kOffsetLinkSpeed ="<< kOffsetLinkSpeed  <<std::endl;
    std::cerr<<"  .LinkStatusUp  ="<< (IsFlagSet(t,kFlagLinkStatusUp) ? '1' : '0')<<std::endl;
    std::cerr<<"  .LINK_SPEED = ";
    switch(t&kMaskLinkSpeed){
        case kValueLinkSpeed100M:  std::cerr<<"100Mb/s"<<std::endl;break;
        case kValueLinkSpeed1G  :  std::cerr<<"  1Gb/s"<<std::endl;break;
        case kValueLinkSpeed10G :  std::cerr<<" 10Gb/s"<<std::endl;break;
        default:  std::cerr<<" ERROR(Reserved value)"<<std::endl;break;
    }
    std::cerr<<"  AllFlags  ="<< std::hex<<std::setfill('0') <<t;
}

void RegRa::show(int idx){
    uint64_t t;
    if(base_addr == NULL){
    std::cerr<<"base_addr is NULL"<<std::endl;
    }
    ReadReg(base_addr,Offset(idx),t);
    std::cerr<<"[Info]RegRa:(MAC address)"<<std::endl;
    std::cerr<<"  kOffset   ="<< kOffset  <<std::endl;
    std::cerr<<"  kNlimit   ="<< kNlimit  <<std::endl;
    HwAddr ad(ntohll(t) >> 16);
    printf("% 3d: %d %s\n", idx, ad.Valid(), ad.FormatAddr().c_str());
    // std::cerr<<"  AllFlags  ="<< std::hex<<std::setfill('0') <<t;
}



















