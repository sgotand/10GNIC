#include<assert.h>
#include<stdint.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>

#include "pcie_uio/generic.h"
#include "pcie_uio/pci.h"
#include "pcie_uio/mem.h"

#include "addr.h"
#include "debug.h"
#include "device.h"

int main(){

	DevNic p;
	p.Init();

	return 0;
}
