#pragma once

#include<cstdint>
#include<cassert>
#include<cstdio>
#include<iostream>

struct Reg{
    static void * base_addr;
    public:
    static void show(){
        std::cerr << "To be defined" <<std::endl;
    };
};
void  *Reg::base_addr = NULL;

// General Control Registers (see p.483)
struct RegCtrl :public Reg {
    static const uint32_t kOffset     = 0x00000000;
    static const uint32_t kFlagLinkReset   = 1<< 3;
    static const uint32_t kFlagDeviceReset = 1<<26;
    static void show();
};

struct RegStatus :public Reg{
    static const uint32_t kOffset     = 0x00000008;
    static void show();
};

// CTRL_EXT
// ESDP
// PHY_GPIO
// MAC_GPIO
// PHYINT_STATUS0
// PHYINT_STATUS1
// PHYINT_STATUS2
// LEDCTL
// EXVET
// EXVET_T
// FACTPS
// GRC
// DEV_FUNC_EN
// I2CCMD
// I2CPARAMS

// NVM Registers (see p.483)
struct RegEec :public Reg{
    static const uint32_t kOffset 		= 0x00010010;
    static const uint32_t kFlagAutoRd	= 1 << 9;
    static void show();
};

// FLA
struct RegEemngctl :public Reg{
    static const uint32_t kOffset 		= 0x00010110;
    static const uint32_t kFlagCfgDone0 = 1<<18;
    static const uint32_t kFlagCfgDone1 = 1<<19;
    static void show();
};
// EEMNGDATA
// FLMNGDATA
// JEDEC_ID_1


// Flow Control Registers
struct RegFcttvn :public Reg{
    static const uint32_t kOffset 	= 0x00003200;
    static const uint32_t kNlimit	= 4;
    static void show();
};
struct RegFcrtl :public Reg{
    static const uint32_t kOffset 	= 0x00003220;
    static const uint32_t kNlimit 	= 8;
    static void show();
};
struct RegFcrth :public Reg{
    static const uint32_t kOffset 	= 0x00003220;
    static const uint32_t kNlimit 	= 8;
    static void show();
};




struct RegLinks :public Reg{
    static const uint32_t kOffset = 0x000042A4;
    static const uint32_t kFlagLinkStatusUp = 1<<7;
    static const uint32_t kOffsetLinkSpeed = 28;
    static const uint32_t kMaskLinkSpeed = 0b11<<28;
    static const uint32_t kValueLinkSpeed100M = 0b01 << 28;
    static const uint32_t kValueLinkSpeed1G = 0b10 << 28;
    static const uint32_t kValueLinkSpeed10G = 0b11 << 28;
    static void show();
};
struct RegEims :public Reg{
    static const uint32_t kOffset     = 0x00000880;
};
struct RegEimc :public Reg{
    static const uint32_t kOffset     = 0x00000888;
};
struct RegRdrxctl :public Reg{
    static const uint32_t kOffset     		= 0x00002F00;
    static const uint32_t kFlagDmaidone 	= 1<<3;
};
struct RegFctrl :public Reg{
    static const uint32_t kOffset     = 0x00005080;
    static const uint32_t kFlagMulticastEnable = 1<<8;
    static const uint32_t kFlagUnicastEnable   = 1<<9;
    static const uint32_t kFlagBroadcastEnable = 1<<10;
};

//Reseive Registers
// Receive Descripter Base address
struct RegRdba :public Reg{
    static const uint32_t kOffset   = 0x00001000;
    static const uint32_t kNlimit   = 128;
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        if(idx < 64)
            return 0x00001000 + 0x40 * idx;
        return 0x0000D000 + 0x40 * (idx - 64);
    }
};
struct RegRdlen :public Reg{
    static const uint32_t kOffset   = 0x00001008;
    static const uint32_t kNlimit   = 128;
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        if(idx < 64)
            return 0x00001008 + 0x40 * idx;
        return 0x0000D008 + 0x40 * (idx - 64);
    }
};
struct RegRdh :public Reg{
    static const uint32_t kOffset   = 0x00001010;
    static const uint32_t kNlimit   = 128;
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        if(idx < 64)
            return 0x00001010 + 0x40 * idx;
        return 0x0000D010 + 0x40 * (idx - 64);
    }
};
struct RegRdt :public Reg{
    static const uint32_t kOffset   = 0x00001018;
    static const uint32_t kNlimit   = 128;
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        if(idx < 64)
            return 0x00001018 + 0x40 * idx;
        return 0x0000D018 + 0x40 * (idx - 64);
    }
};

// Receive Address(low,high)
struct RegRa :public Reg{
    static const uint32_t kOffset   = 0x0000A200;
    static const uint32_t kNlimit   = 128;
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        return 0x0000A200 + 0x8 * idx;
    }
    static void show(int idx);
};
struct RegSrrctl :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        if(idx < 64)
            return 0x00001014 + 0x40 * idx;
        return 0x0000D014 + 0x40 * (idx - 64);
    }
};
struct RegRscctl :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        if(idx < 64)
            return 0x0000102C + 0x40 * idx;
        return 0x0000D02C + 0x40 * (idx - 64);
    }
};
struct RegRxdctl :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        if(idx < 64)
            return 0x00001028 + 0x40 * idx;
        return 0x0000D028 + 0x40 * (idx - 64);
    }
    static const uint32_t kFlagReceiveQueueEnable = 1<<25;
};
struct RegRxctrl :public Reg{
    static const uint32_t kOffset = 0x00003000;
    static const uint32_t kFlagEnable = 1<<0;
};

//Transmit Registers

struct RegDmatxctl :public Reg{
    static const uint32_t kOffset     = 0x00004A80;
    static const uint32_t kFlagTransmitEnable = 1<<0;
};

struct RegDtxtcpflgl :public Reg{
    static const uint32_t kOffset     = 0x00004A88;
};

struct RegDtxtcpflgh :public Reg{
    static const uint32_t kOffset     = 0x00004A8C;
};



struct RegTdba :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        assert(128  > idx && idx >= 0);
        return 0x00006000 + 0x40 * idx;
    }
};

struct RegTdlen :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        assert(128  > idx && idx >= 0);
        return 0x00006008 + 0x40 * idx;
    }
};


struct RegTdh :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        assert(128  > idx && idx >= 0);
        return 0x00006010 + 0x40 * idx;
    }
};

struct RegTdt :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        assert(128  > idx && idx >= 0);
        return 0x00006018 + 0x40 * idx;
    }
};

struct RegTxdctl :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        assert(128  > idx && idx >= 0);
        return 0x00006028 + 0x40 * idx;
    }
};

struct RegTdwba :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        assert(128  > idx && idx >= 0);
        return 0x00006038 + 0x40 * idx;
    }
};

struct RegDtxmxszrq :public Reg{
    static uint32_t Offset() {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        return 0x00008100;
    }
};


struct RegMtqc :public Reg{
    static uint32_t Offset() {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        return 0x00008120;
    }
};

struct RegTxpbsize :public Reg{
    static uint32_t Offset(size_t idx) {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        assert(8  > idx && idx >= 0);
        return 0x0000CC00 + 0x4 * idx;
    }
};

struct RegMngtxmap :public Reg{
    static uint32_t Offset() {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        return 0x0000CD10;
    }
};


struct RegTagetype :public Reg{
    static uint32_t Offset() {
        // FIXME もっと他と同じ感じで書きたいですねぇ
        return 0x00017100;
    }
};

// Read Write Functions
void ReadReg(void *, uint32_t, uint8_t &);
void ReadReg(void *, uint32_t, uint16_t &);
void ReadReg(void *, uint32_t, uint32_t &);
void ReadReg(void *, uint32_t, uint64_t &);

void WriteReg(void *addr, uint32_t off, uint8_t val);
void WriteReg(void *addr, uint32_t off, uint16_t val);
void WriteReg(void *addr, uint32_t off, uint32_t val);
void WriteReg(void *addr, uint32_t off, uint64_t val);
