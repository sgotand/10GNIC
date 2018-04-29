#include <sstream>

class Addr {
	public:
	enum EType
	{
		MAC  = 0,
		ETag = 1,
	};

	Addr(uint64_t raw) {
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
		ss << std::hex;
		ss << ((_low  & 0x000000FF) >>  0) << ":";
		ss << ((_low  & 0x0000FF00) >>  8) << ":";
		ss << ((_low  & 0x00FF0000) >> 16) << ":";
		ss << ((_low  & 0xFF000000) >> 24) << ":";
		ss << ((_high & 0x000000FF) >>  0) << ":";
		ss << ((_high & 0x0000FF00) >>  8);
		return ss.str();
	}



	private:
	Addr(); // hide
	uint32_t _high, _low;

};
