struct Sdhci {
    uint32_t SADDR;      // 0x0000
    uint16_t BLKSIZ;     // 0x0004
    uint16_t BLKCNT;     // 0x0006
    uint32_t ARG;        // 0x0008
    uint16_t TRANSMOD;   // 0x000c
    uint16_t CMD;        // 0x000e
    uint32_t RESP0;      // 0x0010
    uint32_t RESP1;      // 0x0014
    uint32_t RESP2;      // 0x0018
    uint32_t RESP3;      // 0x001c
    uint32_t BUFFER;     // 0x0020
    uint32_t PRESTS;     // 0x0024
    uint8_t  HOSTCTRL1;  // 0x0028
    uint8_t  PWRCTRL;    // 0x0029
    uint8_t  BLKGAPCTRL; // 0x002a
    uint8_t  reserved;            // 0x002b (padding)
    uint16_t CLKCTRL;    // 0x002c
    uint8_t  TIMEOUT;    // 0x002e
    uint8_t  SWRST;      // 0x002f
    uint16_t NORINTSTS;  // 0x0030
    uint16_t ERRINTSTS;      // 0x0032
    uint16_t NORINTSTSE;     // 0x0034
    uint16_t ERRINTSTSE;     // 0x0036
    uint16_t NORINTSIGE;     // 0x0038
    uint16_t ERRINTSIGE;     // 0x003a
    uint16_t ACMDERRSTS;     // 0x003c
    uint16_t HOSTCTRL2;      // 0x003e
    uint32_t CAP;            // 0x0040
    uint8_t reserved[12];
    uint16_t FEACMD;         // 0x0050
    uint16_t FEERRINT;       // 0x0052
    uint16_t ADMAERSTS;      // 0x0054
    uint32_t ADMAADDR;       // 0x0058
    uint8_t reserved[6];
    uint16_t PVALINIT;       // 0x0060
    uint16_t PVALDS;         // 0x0062
    uint16_t PVAHS;          // 0x0064
    uint16_t PVALSDRI2;      // 0x0066
    uint16_t PVALSDR25;      // 0x0068
    uint16_t PVALSDR50;      // 0x006a
    uint16_t PVALSDR104;     // 0x006c
    uint16_t PVALDDR50;      // 0x006e
    uint32_t BOOTTIMEOUT;    // 0x0070
    uint16_t PVALHS400;      // 0x0074
    uint16_t reserved4;
    uint16_t VENDOR;         // 0x0078
    uint8_t reserved4[(16*8) + 2];
    uint16_t SLOTINTSTS;     // 0x00fc
    uint16_t VERSION;        // 0x00fe
    uint8_t reserved4[(16*16)];
    uint32_t CQVER;          // 0x0200
    uint32_t CQCAP;          // 0x0204
    uint32_t CQCFG;          // 0x0208
    uint32_t CQCTRL;         // 0x020c
    uint32_t CQINTSTS;       // 0x0210
    uint32_t CQINTSTSEN;     // 0x0214
    uint32_t CQINTSIGEN;     // 0x0218
};
