#include <stdint.h>

struct Info {
  uint32_t offset;
  const char *str;
};

Info info[] =
  {
    {0x042A4, "LINKS"},
    {0x05080, "FCTRL"},
    {0x04294, "MFLCN"},
    {0x05088, "VLNCTRL"},
    {0x02100, "SRRCTL0"},
    {0x03D00, "FCCFG"},
    {0x04240, "HLREG0"},
    {0x00000, "CTRL"},
    {0x00008, "STATUS"},
    {0x00018, "CTRL_EXT"},
    {0x00020, "ESDP"},
    {0x00028, "EODSDP"},
    {0x00200, "LEDCTL"},
    {0x00048, "FRTIMER"},
    {0x0004C, "TCPTIMER"},
    {0x10010, "EEC"},
    {0x10014, "EERD"},
    {0x1001C, "FLA"},
    {0x10110, "EEMNGCTL"},
    {0x10114, "EEMNGDATA"},
    {0x10118, "FLMNGCTL"},
    {0x1011C, "FLMNGDATA"},
    {0x10120, "FLMNGCNT"},
    {0x1013C, "FLOP"},
    {0x10200, "GRC"},
    {0x00800, "EICR"},
    {0x00808, "EICS"},
    {0x00880, "EIMS"},
    {0x00888, "EIMC"},
    {0x00810, "EIAC"},
    {0x00890, "EIAM"},
    {0x00820, "EITR0"},
    {0x00900, "IVAR0"},
    {0x01000, "RDBAL00"},
    {0x01004, "RDBAH00"},
    {0x01008, "RDLEN00"},
    {0x01010, "RDH00"},
    {0x01018, "RDT00"},
    {0x01028, "RXDCTL00"},
    {0x02100, "SRRCTL00"},
    {0x02200, "DCA_RXCTRL00"},
    {0x02F00, "RDRXCTL"},
    {0x03C00, "RXPBSIZE0"},
    {0x01030, "qprc00"},
  };
