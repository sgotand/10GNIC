void initialize_pci(DevPci &dp);
void initialize_hardware(void *addr);
void*initialize_receive_queue(void *regspace, Udmabuf &physmem, size_t descnum, size_t bufsz);
