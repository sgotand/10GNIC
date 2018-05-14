#pragma once

#include <sstream>
#include <iomanip>

class HwAddr {
	public:
	enum EType
	{
		MAC  = 0,
		ETag = 1,
	};

	HwAddr(uint64_t raw) {
		_high = raw >> 32;
		_low = raw;
	}

	bool Valid() {
		return _high & (1 << 31);
	}

	EType Type() {
		return EType(_high & (1 << 30));
	}

	std::string FormatAddr() {
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		ss << std::setw(2) << ((_high & 0x0000FF00) >>  8) << ":";
		ss << std::setw(2) << ((_high & 0x000000FF) >>  0) << ":";
		ss << std::setw(2) << ((_low  & 0xFF000000) >> 24) << ":";
		ss << std::setw(2) << ((_low  & 0x00FF0000) >> 16) << ":";
		ss << std::setw(2) << ((_low  & 0x0000FF00) >>  8) << ":";
		ss << std::setw(2) << ((_low  & 0x000000FF) >>  0);
		return ss.str();
	}



	private:
	HwAddr(); // hide
	uint32_t _high, _low;

};

class Ip4Addr {
	public:
	Ip4Addr(uint32_t raw) {
		_low = raw;
	}

	std::string Format() {
		std::stringstream ss;
		ss << std::dec;
		ss << ((_low & 0xFF000000) >> 24) << ".";
		ss << ((_low & 0x00FF0000) >> 16) << ".";
		ss << ((_low & 0x0000FF00) >>  8) << ".";
		ss << ((_low & 0x000000FF) >>  0);
		return ss.str();
	}

	private:
	Ip4Addr(); // hide
	uint32_t _low;

};
