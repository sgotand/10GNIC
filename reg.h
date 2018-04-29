#include<cstdint>
#include<cassert>

struct Reg { };
struct RegCtrl : Reg {
	static const uint32_t kOffset     = 0x00000000;

	static const uint32_t kFlagDeviceReset = 1<<26;
};

struct RegStatus {
	static const uint32_t kOffset     = 0x00000008;
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
struct RegEims {
	static const uint32_t kOffset     = 0x00000880;
};

void ReadReg(void *addr, uint32_t off, uint64_t &val) {
	assert(false);
}
void ReadReg(void *addr, uint32_t off, uint32_t &val) {
	val = ((uint32_t *)addr)[off/4];		
}
void ReadReg(void *addr, uint32_t off, uint16_t &val) {
	val = ((uint16_t *)addr)[off/2];		
}

void ReadReg(void *addr, uint32_t off, uint8_t &val) {
	val = ((uint8_t *)addr)[off];		
}

void WriteReg(void *addr, uint32_t off, uint64_t &val) {
	assert(false);
}
void WriteReg(void *addr, uint32_t off, uint32_t val) {
	((uint32_t *)addr)[off/4] = val;		
}

void WriteReg(void *addr, uint32_t off, uint16_t val) {
	((uint16_t *)addr)[off/2] = val;		
}

void WriteReg(void *addr, uint32_t off, uint8_t val) {
	((uint8_t *)addr)[off] = val;		
}
