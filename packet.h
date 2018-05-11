#ifndef PACKET__
#define PACKET__
//#include <iostream>
#include <cstdint>
#include <net/ethernet.h>
//定数等はこのヘッダファイルに記述あり。


//送受信両方扱えるように書きたい。


class MacAddr{
	public: 
	uint8_t addr[6];

	MacAddr(void){
		for(int i = 0 ; i < 6 ; i++)addr[i] = 0;
	}

	MacAddr(MacAddr &origin){
		this->set(origin);
	}

	MacAddr(const char *str){
		this->set(str);
	}

	void set(MacAddr &origin){
		for(int i = 0;i<6;i++) addr[i] = origin.addr[i];
	}

	void set(const char  *str){
		for(int i=0;i<12;i++){
			if(i%2 == 0)	addr[i/2] = 0;
			else 		addr[i/2] <<=4;


			if	('0' <= *str && *str <='9')	addr[i/2] +=*str -'0';
			else if	('a' <= *str && *str <='f') 	addr[i/2] +=*str -'a' + 10;
			else if	('A' <= *str && *str <='F')	addr[i/2] +=*str -'A' + 10;
			else{
				std::cerr << "EROOR@" << __PRETTY_FUNCTION__ << std::endl;
				std::cerr << "指定されたアドレスが不正です。"<< std::endl;
			}
			str++;
		}
	}

	void print(void){
		for(int i = 0; i < 6 ; i++){
			std::printf("%02x",addr[i]);
			if(i<5) std::putchar(':');
			else std::putchar('\n');
		}
	}
};


class Packet{

	private:
	void *buffer_addr;

	public:
	//プリアンブルとSFDはNICがつけてくれそう。(要出典)
	MacAddr	dst_mac_addr;
	MacAddr src_mac_addr;
	uint16_t type;

	Packet(){
		buffer_addr
	
	}

	//リトルエンディアンのままなので、設定時に変換する必要あり。
	//static const uint16_t kTypeIpv4 = 0x0800;
	//static const uint16_t kTypeArp  = 0x0806; 
	//static const uint16_t kTypeRarp = 0x8035; 
	//static const uint16_t kTypeAppleTalk = 0x809b; 
	//static const uint16_t kTypeIeee8021q = 0x8100; 
	//static const uint16_t kTypeNetWareIpx = 0x8137;
	//static const uint16_t kTypeIpv6 = 0x86dd; 
	//static const uint16_t kTypeIeee8021x = 0x888e; 
};



class Arp: public Packet{
	public:
	void init(void){
		//dst_mac_addrを設定
		//src_mac_addrを設定(自動取得したい.)		
		//type = kTypeArp;
		
		Packet::dst_mac_addr.set("FFFFFFFFFFFF");
		Packet::src_mac_addr.set("A0369FF2E419");
		

		printf("dst_mac_addr = ");
		Packet::dst_mac_addr.print();

		printf("src_mac_addr = ");
		Packet::src_mac_addr.print();
		
		type = ETHERTYPE_ARP;
	}


};
	
	



#endif
