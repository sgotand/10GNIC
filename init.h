#include "queue.h"

void initialize_pci(DevPci &dp);
void initialize_hardware(void *addr);
ReceiveQueue* initialize_receive(void *regspace, phys_addr physbase, void *virtbase, size_t descnum, size_t bufsz);
TransmitQueue* initialize_transmit(void *regspace, phys_addr physbase, void *virtbase, size_t descnum , size_t bufsz);
