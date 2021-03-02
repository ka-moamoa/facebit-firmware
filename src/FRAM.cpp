#include "FRAM.h"
#include "rtos.h"

// https://www.cypress.com/documentation/application-notes/an304-spi-guide-f-ram


FRAM::FRAM(SPI *spi, PinName fram_cs) :
_fram_cs(fram_cs)
{
    _logger = Logger::get_instance();
    _spi = spi;
    _fram_cs = 1;
}

FRAM::~FRAM()
{
}

void FRAM::initialize()
{
}


bool FRAM::write_bytes(uint32_t address, const char *tx_buffer, int tx_bytes)
{
    if (address > MAX_ADDRESS)
    {
        _logger->log(TRACE_WARNING, "%s", "SPI address is too large!");
        return false;
    }

    Unpacker unpack;
    unpack.address = address;

    const char write_address[3] = {unpack.bytes[2], unpack.bytes[1], unpack.bytes[0]}; // flip endianness
    
    _fram_cs = 0; // CS assert
    _write_opcode(OP_WREN);
    _fram_cs = 1; //CS deassert (CS has to toggle after the WREN opcode)
    
    _fram_cs = 0;
    _spi->write(OP_WRITE); // Send write opcode
    _spi->write(write_address, 3, NULL, 0); // Send write address
    _spi->write(tx_buffer, tx_bytes, NULL, 0); // This is the actual data transmission
    _fram_cs = 1; // CS deassert

    return true; // TODO add error checking
}

uint8_t FRAM::read_bytes(uint32_t address, char *rx_buffer, int rx_bytes)
{
    if (address > MAX_ADDRESS)
    {
        _logger->log(TRACE_WARNING, "%s", "SPI address is too large!");
        return false;
    }

    Unpacker unpack;
    unpack.address = address;

    const char read_address[3] = {unpack.bytes[2], unpack.bytes[1], unpack.bytes[0]}; // flip endianness

    _fram_cs = 0; // CS assert
    
    _spi->write(OP_READ); // send read opcode
    _spi->write(read_address, 3, NULL, 0); // send read address
    _spi->write(NULL, 0, rx_buffer, rx_bytes); // read out data

    _fram_cs = 1; // CS deassert

     return true; // TODO add SPI error checking
}

bool FRAM::_write_opcode(uint8_t opcode)
{
    _spi->write(opcode);
    return true; // TODO add spi error checking
};