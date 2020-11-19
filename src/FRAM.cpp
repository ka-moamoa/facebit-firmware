#include "FRAM.h"
#include "rtos.h"
#include "logger.h"

FRAM::FRAM(PinName fram_vcc, PinName fram_mosi, PinName fram_miso, PinName fram_sck, PinName fram_cs)
:
_spi(fram_mosi, fram_miso, fram_sck),
_fram_cs(fram_cs),
_fram_vcc(fram_vcc)
{
}

FRAM::~FRAM()
{
}

void FRAM::turn_on()
{
    _fram_vcc.write(1); // power on
    _fram_cs.write(1); // CS deassert
    ThisThread::sleep_for(1ms); // respect minimum t_pu (250 us)
}

void FRAM::turn_off()
{
    _fram_vcc.write(0); // power off
    _fram_cs.write(0);
}

bool FRAM::write_bytes(uint32_t address, const char *tx_buffer, int tx_bytes)
{
    if (address > MAX_ADDRESS)
    {
        LOG_WARNING("%s", "SPI address is too large!");
        return false;
    }

    char *p = (char *) &address;

    // prepends the "write" opcode to the address
    char opcode_address[4] = {WRITE, p[0], p[1], p[2]};
    
    _fram_cs.write(0); // CS assert
    _write_opcode(WREN);
    _fram_cs.write(1); //CS deassert (datasheet leads me to believe CS has to toggle after the WREN opcode)
    
    _fram_cs.write(0);
    _spi.write(opcode_address, 4, NULL, 0); // This sends the WRITE command along with the 17-bit address
    _spi.write(tx_buffer, tx_bytes, NULL, 0); // This is the actual data transmission
    _fram_cs.write(1); // CS deassert
}

uint8_t FRAM::read_bytes(uint32_t address, char *rx_buffer, int rx_bytes)
{
    _fram_cs.write(0); // CS assert
    
    char *p = (char *) &address;

    char opcode_address[4] = {READ, p[0], p[1], p[2]};
    
    _spi.write(opcode_address, 4, rx_buffer, 4 + rx_bytes);

    _fram_cs.write(1); // CS deassert
}

bool FRAM::_write_opcode(uint8_t opcode)
{
    _spi.write(opcode);
};