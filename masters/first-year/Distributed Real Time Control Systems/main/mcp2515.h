#ifndef _MCP2515_PICO_H_
#define _MCP2515_PICO_H_

// EFLG register (0x2D) bit masks
// Copied from slides 
#define EFLG_RX1OVR  0x80   // RX Buffer 1 Overflow
#define EFLG_RX0OVR  0x40   // RX Buffer 0 Overflow
#define EFLG_TXBO    0x20   // Bus-Off Error
#define EFLG_TXEP    0x10   // TX Error-Passive
#define EFLG_RXEP    0x08   // RX Error-Passive
#define EFLG_TXWAR   0x04   // TX Error Warning (TEC >= 96)
#define EFLG_RXWAR   0x02   // RX Error Warning (REC >= 96)
#define EFLG_EWARN   0x01   // Error Warning (TEC or REC >= 96)

#include <Arduino.h>
#include "hardware/spi.h" // Use Pico SDK SPI

struct can_frame {
    uint32_t can_id;
    uint8_t can_dlc;
    uint8_t data[8];
};

enum CAN_SPEED { CAN_1000KBPS, CAN_500KBPS, CAN_250KBPS, CAN_125KBPS };

class MCP2515 {
public:
    enum ERROR { ERROR_OK, ERROR_FAIL, ERROR_ALLTXBUSY, ERROR_FAILINIT, ERROR_FAILTX, ERROR_NOMSG };

    // Argument 1 is now spi_inst_t* (matches 'spi0')
    MCP2515(spi_inst_t* spi, uint8_t cs, uint8_t tx, uint8_t rx, uint8_t sck, uint32_t clk_speed);

    ERROR reset(void);
    ERROR setNormalMode();
    ERROR setLoopbackMode();
    ERROR setConfigMode();
    ERROR setBitrate(const CAN_SPEED canSpeed);
    ERROR sendMessage(const struct can_frame *frame);
    ERROR readMessage(struct can_frame *frame);

    uint8_t errorCountTX();       // returns TEC register (0x1C)
    uint8_t errorCountRX();       // returns REC register (0x1D)
    uint8_t getErrorFlags();      // returns EFLG register (0x2D)
    void    clearRXnOVRFlags();   // clears RX0OVR and RX1OVR in EFLG

private:
    spi_inst_t* _spi_hw;
    uint8_t _cs, _tx, _rx, _sck;
    uint32_t _clk;
    void writeReg(uint8_t reg, uint8_t val);
    uint8_t readReg(uint8_t reg);
    void modifyReg(uint8_t reg, uint8_t mask, uint8_t data);
};

#endif