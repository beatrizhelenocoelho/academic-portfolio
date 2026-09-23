#include "mcp2515.h"
#include "hardware/gpio.h"

MCP2515::MCP2515(spi_inst_t* spi, uint8_t cs, uint8_t tx, uint8_t rx, uint8_t sck, uint32_t clk_speed) 
    : _spi_hw(spi), _cs(cs), _tx(tx), _rx(rx), _sck(sck), _clk(clk_speed) {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
}

MCP2515::ERROR MCP2515::reset(void) {
    spi_init(_spi_hw, _clk);
    gpio_set_function(_rx, GPIO_FUNC_SPI);
    gpio_set_function(_tx, GPIO_FUNC_SPI);
    gpio_set_function(_sck, GPIO_FUNC_SPI);
    
    digitalWrite(_cs, LOW);
    uint8_t cmd = 0xC0; // Reset
    spi_write_blocking(_spi_hw, &cmd, 1);
    digitalWrite(_cs, HIGH);
    delay(10);
    return ERROR_OK;
}

MCP2515::ERROR MCP2515::setBitrate(const CAN_SPEED canSpeed) {
    setConfigMode();
    // 1000KBPS @ 8MHz Configuration
    writeReg(0x28, 0x00); // CNF1
    writeReg(0x29, 0x80); // CNF2
    writeReg(0x2A, 0x80); // CNF3
    return ERROR_OK;
}

MCP2515::ERROR MCP2515::setNormalMode() {
    modifyReg(0x0F, 0xE0, 0x00);
    return ERROR_OK;
}

MCP2515::ERROR MCP2515::setLoopbackMode() {
    modifyReg(0x0F, 0xE0, 0x40);
    return ERROR_OK;
}

MCP2515::ERROR MCP2515::setConfigMode() {
    modifyReg(0x0F, 0xE0, 0x80);
    return ERROR_OK;
}

MCP2515::ERROR MCP2515::sendMessage(const struct can_frame *frame) {
    // Check TXB0, TXB1, TXB2 in order — use the first one that is not pending transmission.
    // TXBnCTRL registers: 0x30 (TXB0), 0x40 (TXB1), 0x50 (TXB2). TXREQ = bit 3.
    // Load TX buffer SPI commands (point to TXBnSIDH):
    //   0x40 = TXB0,  0x42 = TXB1,  0x44 = TXB2
    // RTS SPI commands:
    //   0x81 = TXB0,  0x82 = TXB1,  0x84 = TXB2
    static const uint8_t ctrl_regs[3]  = { 0x30, 0x40, 0x50 };
    static const uint8_t load_cmds[3]  = { 0x40, 0x42, 0x44 };
    static const uint8_t rts_cmds[3]   = { 0x81, 0x82, 0x84 };

    int buf = -1;
    for (int i = 0; i < 3; i++) {
        if (!(readReg(ctrl_regs[i]) & 0x08)) { // TXREQ bit clear = buffer free
            buf = i;
            break;
        }
    }
    if (buf < 0) return ERROR_ALLTXBUSY;

    uint8_t header[5] = {
        (uint8_t)(frame->can_id >> 3),
        (uint8_t)(frame->can_id << 5),
        0, 0,
        frame->can_dlc
    };

    digitalWrite(_cs, LOW);
    spi_write_blocking(_spi_hw, &load_cmds[buf], 1);
    spi_write_blocking(_spi_hw, header, 5);
    spi_write_blocking(_spi_hw, frame->data, frame->can_dlc);
    digitalWrite(_cs, HIGH);

    digitalWrite(_cs, LOW);
    spi_write_blocking(_spi_hw, &rts_cmds[buf], 1);
    digitalWrite(_cs, HIGH);

    return ERROR_OK;
}

MCP2515::ERROR MCP2515::readMessage(struct can_frame *frame) {
    // CANINTF (0x2C): bit 0 = RXB0 full, bit 1 = RXB1 full.
    // Read RX buffer SPI commands: 0x90 = RXB0, 0x94 = RXB1.
    // Clear flags by writing 0 back to the corresponding CANINTF bit.
    uint8_t canintf = readReg(0x2C);

    uint8_t read_cmd;
    uint8_t clear_mask;

    if (canintf & 0x01) {           // RXB0 has a message — drain it first (higher priority)
        read_cmd   = 0x90;
        clear_mask = 0x01;
    } else if (canintf & 0x02) {    // RXB1 has a message
        read_cmd   = 0x94;
        clear_mask = 0x02;
    } else {
        return ERROR_NOMSG;
    }

    digitalWrite(_cs, LOW);
    spi_write_blocking(_spi_hw, &read_cmd, 1);
    uint8_t buf[5];
    spi_read_blocking(_spi_hw, 0, buf, 5);
    frame->can_id  = ((uint32_t)buf[0] << 3) | (buf[1] >> 5);
    frame->can_dlc = buf[4] & 0x0F;
    spi_read_blocking(_spi_hw, 0, frame->data, frame->can_dlc);
    digitalWrite(_cs, HIGH);

    modifyReg(0x2C, clear_mask, 0x00); // Clear only the flag we just consumed
    return ERROR_OK;
}

void MCP2515::writeReg(uint8_t reg, uint8_t val) {
    uint8_t data[3] = {0x02, reg, val};
    digitalWrite(_cs, LOW);
    spi_write_blocking(_spi_hw, data, 3);
    digitalWrite(_cs, HIGH);
}

uint8_t MCP2515::readReg(uint8_t reg) {
    uint8_t cmd[2] = {0x03, reg};
    uint8_t val;
    digitalWrite(_cs, LOW);
    spi_write_blocking(_spi_hw, cmd, 2);
    spi_read_blocking(_spi_hw, 0, &val, 1);
    digitalWrite(_cs, HIGH);
    return val;
}

void MCP2515::modifyReg(uint8_t reg, uint8_t mask, uint8_t data) {
    uint8_t cmd[4] = {0x05, reg, mask, data};
    digitalWrite(_cs, LOW);
    spi_write_blocking(_spi_hw, cmd, 4);
    digitalWrite(_cs, HIGH);
}

uint8_t MCP2515::errorCountTX() {
    return readReg(0x1C); // TEC register
}

uint8_t MCP2515::errorCountRX() {
    return readReg(0x1D); // REC register
}

uint8_t MCP2515::getErrorFlags() {
    return readReg(0x2D); // EFLG register
}

void MCP2515::clearRXnOVRFlags() {
    modifyReg(0x2D, EFLG_RX0OVR | EFLG_RX1OVR, 0x00);
}
