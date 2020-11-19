#ifndef FRAM_H_
#define FRAM_H_

#include "mbed.h"
#include "SPI.h"
#include "DigitalOut.h"

class FRAM
{
private:
    SPI _spi;
    DigitalOut _fram_cs;
    DigitalOut _fram_vcc;

    bool _write_opcode(uint8_t opcode);

public:
    FRAM(PinName fram_vcc, PinName fram_mosi, PinName fram_miso, PinName fram_sck, PinName fram_cs);
    ~FRAM();

    void turn_on();
    void turn_off();

    bool write_bytes(uint32_t address, const char *tx_buffer, int tx_bytes);
    uint8_t read_bytes(uint32_t address, char *rx_buffer, int rx_bytes);


    const uint8_t WREN    = 0b00000110; // Set write enable latch
    const uint8_t WRDI    = 0b00000100; // Reset write enable latch
    const uint8_t RDSR    = 0b00000101; // Read Status Register
    const uint8_t WRSR    = 0b00000001; // Write Status Register
    const uint8_t READ    = 0b00000011; // Read memory data
    const uint8_t FSTRD   = 0b00001011; // Fast read memory data
    const uint8_t WRITE   = 0b00000010; // Write memory data
    const uint8_t SLEEP   = 0b10111001; // Enter sleep mode
    const uint8_t RDID    = 0b10011111; // Read device ID 
    const uint8_t SNR     = 0b11000011; // Read S/N

    const uint32_t MAX_ADDRESS = 0x0001FFFF;
};





#endif // FRAM_H_