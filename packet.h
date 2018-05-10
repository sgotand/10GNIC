#include <cstdint>
#include <net/ehternet.h>
//定数等はこのヘッダファイルに記述あり。


//送受信両方扱えるように書きたい。
//

class Packet{

	//プリアンブルとSFDはNICがつけてくれそう。(要出典)
	uint8_t	dst_mac_addr[6];
	uint8_t src_mac_addr[6];
	uint16_t type;

	//リトルエンディアンのままなので、設定時に変換する必要あり。
	//static const uint16_t kTypeIpv4 = 0x0800;
	//static const uint16_t kTypeArp  = 0x0806; 
	//static const uint16_t kTypeRarp = 0x8035; 
	//static const uint16_t kTypeAppleTalk = 0x809b; 
	//static const uint16_t kTypeIeee8021q = 0x8100; 
	//static const uint16_t kTypeNetWareIpx = 0x8137;
	//static const uint16_t kTypeIpv6 = 0x86dd; 
	//static const uint16_t kTypeIeee8021x = 0x888e; 
	

}



class Arp: public Packet{


	Arp(){
	
		//type = kTypeArp;
		type = ETHERTYPE_ARP;
	}


}
	
	




