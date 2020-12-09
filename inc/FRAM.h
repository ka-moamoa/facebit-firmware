#ifndef FRAM_H_
#define FRAM_H_

#include "mbed.h"
#include "SPI.h"

class FRAM
{
private:
    SPI *_spi;
    DigitalOut _fram_cs;

    bool _write_opcode(uint8_t opcode);

    typedef union
    {
        uint32_t address;
        uint8_t bytes[4]; // little endian, so [3] gives MSB
    } Unpacker;

    const uint8_t OP_WREN    = 0b00000110; // Set write enable latch
    const uint8_t OP_RDSR    = 0b00000101; // Read Status Register
    const uint8_t OP_WRDI    = 0b00000100; // Reset write enable latch
    const uint8_t OP_WRSR    = 0b00000001; // Write Status Register
    const uint8_t OP_READ    = 0b00000011; // Read memory data
    const uint8_t OP_FSTRD   = 0b00001011; // Fast read memory data
    const uint8_t OP_WRITE   = 0b00000010; // Write memory data
    const uint8_t OP_SLEEP   = 0b10111001; // Enter sleep mode
    const uint8_t OP_RDID    = 0b10011111; // Read device ID 
    const uint8_t OP_SNR     = 0b11000011; // Read S/N

    const uint32_t MAX_ADDRESS = 0x0001FFFF;
public:
    FRAM(SPI *spi, PinName fram_cs);
    ~FRAM();

    bool write_bytes(uint32_t address, const char *tx_buffer, int tx_bytes);
    uint8_t read_bytes(uint32_t address, char *rx_buffer, int rx_bytes);
};





#endif // FRAM_H_