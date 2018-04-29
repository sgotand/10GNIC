#include<cstdint>
#include<cassert>
#include<cstdio>

struct Reg { };
struct RegCtrl : Reg {
	static const uint32_t kOffset     = 0x00000000;

	static const uint32_t kFlagLinkReset   = 1<< 3;
	static const uint32_t kFlagDeviceReset = 1<<26;
};

struct RegStatus {
	static const uint32_t kOffset     = 0x00000008;
};
struct RegLinks {
	static const uint32_t kOffset = 0x000042A4;
	static const uint32_t kFlagLinkStatusUp = 1<<7;
	static const uint32_t kOffsetLinkSpeed = 28;
	static const uint32_t kMaskLinkSpeed = 0b11<<28;
	static const uint32_t kValueLinkSpeed100M = 0b01 << 28;
	static const uint32_t kValueLinkSpeed1G = 0b10 << 28;
	static const uint32_t kValueLinkSpeed10G = 0b11 << 28;
};
struct RegEims {
	static const uint32_t kOffset     = 0x00000880;
};
struct RegEimc {
	static const uint32_t kOffset     = 0x00000888;
};
struct RegRdrxctl {
	static const uint32_t kOffset     = 0x00002F00;
};
struct RegEemngctl {
	static const uint32_t kOffset     = 0x00010110;
};
struct RegFctrl {
	static const uint32_t kOffset     = 0x00005080;
	static const uint32_t kFlagMulticastEnable = 1<<8;
	static const uint32_t kFlagUnicastEnable   = 1<<9;
	static const uint32_t kFlagBroadcastEnable = 1<<10;
};

struct RegRdba {
	static uint32_t Offset(size_t idx) {
		// FIXME もっと他と同じ感じで書きたいですねぇ
		if(idx < 64)
			return 0x00001000 + 0x40 * idx;
		return 0x0000D000 + 0x40 * (idx - 64);
	}
};
struct RegRdlen {
	static uint32_t Offset(size_t idx) {
		// FIXME もっと他と同じ感じで書きたいですねぇ
		if(idx < 64)
			return 0x00001008 + 0x40 * idx;
		return 0x0000D008 + 0x40 * (idx - 64);
	}
};
struct RegRdh {
	static uint32_t Offset(size_t idx) {
		// FIXME もっと他と同じ感じで書きたいですねぇ
		if(idx < 64)
			return 0x00001010 + 0x40 * idx;
		return 0x0000D010 + 0x40 * (idx - 64);
	}
};
struct RegRdt {
	static uint32_t Offset(size_t idx) {
		// FIXME もっと他と同じ感じで書きたいですねぇ
		if(idx < 64)
			return 0x00001018 + 0x40 * idx;
		return 0x0000D018 + 0x40 * (idx - 64);
	}
};
struct RegRa {
	static uint32_t Offset(size_t idx) {
		// FIXME もっと他と同じ感じで書きたいですねぇ
		return 0x0000A200 + 0x8 * idx;
	}
};
struct RegSrrctl {
	static uint32_t Offset(size_t idx) {
		// FIXME もっと他と同じ感じで書きたいですねぇ
		if(idx < 64)
			return 0x00001014 + 0x40 * idx;
		return 0x0000D014 + 0x40 * (idx - 64);
	}
};
struct RegRscctl {
	static uint32_t Offset(size_t idx) {
		// FIXME もっと他と同じ感じで書きたいですねぇ
		if(idx < 64)
			return 0x0000102C + 0x40 * idx;
		return 0x0000D02C + 0x40 * (idx - 64);
	}
};
struct RegRxdctl {
	static uint32_t Offset(size_t idx) {
		// FIXME もっと他と同じ感じで書きたいですねぇ
		if(idx < 64)
			return 0x00001028 + 0x40 * idx;
		return 0x0000D028 + 0x40 * (idx - 64);
	}
	static const uint32_t kFlagReceiveQueueEnable = 1<<25;
};
struct RegRxctrl {
	static const uint32_t kOffset = 0x00003000;
	static const uint32_t kFlagEnable = 1<<0;
};

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
	__asm__ volatile("":::"memory");
	uint8_t t;
	ReadReg(addr, off, t);
	printf(" 8 %08x %08x %08x\n", off, val, t);
}
void WriteReg(void *addr, uint32_t off, uint16_t val) {
	((uint16_t *)addr)[off/2] = val;		
	__asm__ volatile("":::"memory");
	uint16_t t;
	ReadReg(addr, off, t);
	printf("16 %08x %08x %08x\n", off, val, t);
}
void WriteReg(void *addr, uint32_t off, uint32_t val) {
	((uint32_t *)addr)[off/4] = val;		
	__asm__ volatile("":::"memory");
	uint32_t t;
	ReadReg(addr, off, t);
	printf("32 %08x %08x %08x\n", off, val, t);
}
void WriteReg(void *addr, uint32_t off, uint64_t val) {
	WriteReg(addr, off, (uint32_t)(val & 0xFFFFFFFF));
	WriteReg(addr, off+4, (uint32_t)(val >> 32));
	__asm__ volatile("":::"memory");
	uint64_t t;
	ReadReg(addr, off, t);
	printf("64 %08x %016lx %016lx\n", off, val, t);
}
