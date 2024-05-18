#ifndef __RSP_H__
#define __RSP_H__

#include "rsp_vu.h"
#include "recomp.h"
#include <cstdio>

enum class RspExitReason {
    Invalid,
    Broke,
    ImemOverrun,
    UnhandledJumpTarget,
    Unsupported
};

extern uint8_t dmem[];
extern uint16_t rspReciprocals[512];
extern uint16_t rspInverseSquareRoots[512];

#define RSP_MEM_B(offset, addr) \
    (*reinterpret_cast<int8_t*>(dmem + (0xFFF & (((offset) + (addr)) ^ 3))))

#define RSP_MEM_BU(offset, addr) \
    (*reinterpret_cast<uint8_t*>(dmem + (0xFFF & (((offset) + (addr)) ^ 3))))

static inline uint32_t RSP_MEM_W_LOAD(uint32_t offset, uint32_t addr) {
    uint32_t out;
    for (int i = 0; i < 4; i++) {
        reinterpret_cast<uint8_t*>(&out)[i ^ 3] = RSP_MEM_BU(offset + i, addr);
    }
    return out;
}

static inline void RSP_MEM_W_STORE(uint32_t offset, uint32_t addr, uint32_t val) {
    for (int i = 0; i < 4; i++) {
        RSP_MEM_BU(offset + i, addr) = reinterpret_cast<uint8_t*>(&val)[i ^ 3];
    }
}

static inline uint32_t RSP_MEM_HU_LOAD(uint32_t offset, uint32_t addr) {
    uint16_t out;
    for (int i = 0; i < 2; i++) {
        reinterpret_cast<uint8_t*>(&out)[(i + 2) ^ 3] = RSP_MEM_BU(offset + i, addr);
    }
    return out;
}

static inline uint32_t RSP_MEM_H_LOAD(uint32_t offset, uint32_t addr) {
    int16_t out;
    for (int i = 0; i < 2; i++) {
        reinterpret_cast<uint8_t*>(&out)[(i + 2) ^ 3] = RSP_MEM_BU(offset + i, addr);
    }
    return out;
}

static inline void RSP_MEM_H_STORE(uint32_t offset, uint32_t addr, uint32_t val) {
    for (int i = 0; i < 2; i++) {
        RSP_MEM_BU(offset + i, addr) = reinterpret_cast<uint8_t*>(&val)[(i + 2) ^ 3];
    }
}

#define RSP_ADD32(a, b) \
    ((int32_t)((a) + (b)))

#define RSP_SUB32(a, b) \
    ((int32_t)((a) - (b)))

#define RSP_SIGNED(val) \
    ((int32_t)(val))

#define SET_DMA_DMEM(dmem_addr) dma_dmem_address = (dmem_addr)
#define SET_DMA_DRAM(dram_addr) dma_dram_address = (dram_addr)
#define DO_DMA_READ(sp_dma_rdlen) dma_rdram_to_dmem(rdram, dma_dmem_address, dma_dram_address, (sp_dma_rdlen))
#define DO_DMA_WRITE(sp_dma_wr_len) dma_dmem_to_rdram(rdram, dma_dmem_address, dma_dram_address, (sp_dma_wr_len))

static inline void dma_rdram_to_dmem(uint8_t* rdram, uint32_t dmem_addr, uint32_t dram_addr, uint32_t sp_dma_rdlen) {
    uint32_t skip = (sp_dma_rdlen >> 20) & 0xFF8;
    uint32_t count = (sp_dma_rdlen >> 12) & 0xFF;
    uint32_t rd_len = (sp_dma_rdlen >> 0) & 0xFF8;

    count += 1; // Count is inclusive

    rd_len += 1; // Write length is inclusive
    rd_len = (rd_len + 7) & ~7;

    dram_addr &= 0xFFFFF8;
    assert(dmem_addr + rd_len <= 0x1000);
    for (uint32_t row_number = 0; row_number < count; row_number++) {
        for (uint32_t i = 0; i < rd_len; i++) {
            RSP_MEM_B(i, dmem_addr) = MEM_B(0, (int64_t)(int32_t)(dram_addr + i + 0x80000000));
        }

        rdram += skip;
    }
}

static inline void dma_dmem_to_rdram(uint8_t* rdram, uint32_t dmem_addr, uint32_t dram_addr, uint32_t sp_dma_wr_len) {
    uint32_t skip = (sp_dma_wr_len >> 20) & 0xFF8;
    uint32_t count = (sp_dma_wr_len >> 12) & 0xFF;
    uint32_t wr_len = (sp_dma_wr_len >> 0) & 0xFF8;

    count += 1; // Count is inclusive

    wr_len += 1; // Write length is inclusive
    wr_len = (wr_len + 7) & ~7;

    // fprintf(stderr, "\n");
    // fprintf(stderr, "dmem_addr: 0x%08X\n", dmem_addr);
    // fprintf(stderr, "sp_dma_wr_len: 0x%08X\n", sp_dma_wr_len);
    // fprintf(stderr, "  skip: %i\n", skip);
    // fprintf(stderr, "  count: %i\n", count);
    // fprintf(stderr, "  wr_len: %i\n", wr_len);
    // fprintf(stderr, "\n");

    dram_addr &= 0xFFFFF8;
    assert(dmem_addr + wr_len <= 0x1000);
    for (uint32_t row_number = 0; row_number < count; row_number++) {
        for (uint32_t i = 0; i < wr_len; i++) {
            MEM_B(0, (int64_t)(int32_t)(dram_addr + i + 0x80000000)) = RSP_MEM_B(i, dmem_addr);
        }

        rdram += skip;
    }
}

#endif
