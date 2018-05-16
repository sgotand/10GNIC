#pragma once

#include<stdlib.h>
#include<sstream>
#include <net/ethernet.h>
//できれば net/ethernet.h をできるだけ使いたい...

class EtherReader {
	public:
		enum EType {
			Unknown = 0,
			IPv4,
			ARP,
		};

		EtherReader(const void *buf) {
			_buf = (uint8_t*)buf;
		}
		HwAddr Dst() {
			uint64_t raw = 0;
			for(int i=0; i<6; i++) {
				raw <<= 8;
				raw |= _buf[i];
			}
			return HwAddr(raw);
		}
		HwAddr Src() {
			uint64_t raw = 0;
			for(int i=0; i<6; i++) {
				raw <<= 8;
				raw |= _buf[0x6+i];
			}
			return HwAddr(raw);
		}
		EType Type() {
			const size_t typeOffset = 0xc;
			const uint16_t type = *(const uint16_t*)(_buf+typeOffset);
			switch(ntohs(type)) {
				//case 0x0800:
				case ETHERTYPE_IP:	
					return IPv4;
				//case 0x0806:
				case ETHERTYPE_ARP:
					return ARP;
				default:
					return Unknown;
			}
		}
		const void* Data() {
			const size_t dataOffset = 14;
			return _buf+dataOffset;
		}
		static std::string Format(EType t) {
			switch(t) {
				case IPv4:
					return std::string("IPv4");
				case ARP:
					return std::string("ARP");
				default:
					return std::string("(unknown)");

			}
		}
	private:
		EtherReader();
		const uint8_t *_buf;
};

class ArpReader {
	public:
		ArpReader(const void *buf) {
			_buf = (uint8_t*)buf;
		}
		HwAddr HwSrc() {
			uint64_t raw = 0;
			for(int i=0; i<6; i++) {
				raw <<= 8;
				raw |= _buf[0x08+i];
			}
			return HwAddr(raw);
		}
		Ip4Addr IpSrc() {
			uint32_t raw = 0;
			for(int i=0; i<4; i++) {
				raw <<= 8;
				raw |= _buf[14+i];
			}
			return Ip4Addr(raw);
		}
		HwAddr HwDst() {
			uint64_t raw = 0;
			for(int i=0; i<6; i++) {
				raw <<= 8;
				raw |= _buf[0x12+i];
			}
			return HwAddr(raw);
		}
		Ip4Addr IpDst() {
			uint32_t raw = 0;
			for(int i=0; i<4; i++) {
				raw <<= 8;
				raw |= _buf[0x18+i];
			}
			return Ip4Addr(raw);
		}
	private:
		ArpReader();
		const uint8_t *_buf;
};

class IpReader {
	public:
		IpReader(const void *buf) {
			_buf = (uint8_t*)buf;
		}
		Ip4Addr Src() {
			const size_t srcOffset = 12;

			uint32_t raw = 0;
			for(int i=0; i<4; i++) {
				raw <<= 8;
				raw |= _buf[srcOffset+i];
			}
			return Ip4Addr(raw);
		}
		Ip4Addr Dst() {
			const size_t dstOffset = 16;
			uint32_t raw = 0;
			for(int i=0; i<4; i++) {
				raw <<= 8;
				raw |= _buf[dstOffset+i];
			}
			return Ip4Addr(raw);
		}
		const void* Data() {
			// TODO オプションフィールド～～～
			return _buf+20;
		}
	private:
		IpReader();
		const uint8_t *_buf;
};
