#include<cstdint>
#include<cassert>
#include<cstdio>

#include "reg.h"

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
