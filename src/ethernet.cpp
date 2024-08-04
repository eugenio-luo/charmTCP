#include <iostream>

#include "ethernet.hpp"
#include "tun.hpp"
#include "memorypool.hpp"
#include "types.hpp"

static Memory::ObjectPool<Ethernet::Packet> packetsPool{};

CRC32 Ethernet::calcCRC(CRC32 crc, void *buffer, size_t bufferLength) 
{
    unsigned char *data = static_cast<unsigned char*>(buffer);
    crc ^= 0xffffffff;
    while (bufferLength--) {
        crc ^= *data++;
        for (unsigned k = 0; k < 8; ++k) {
            crc = crc & 1 ? (crc >> 1) ^ 0xedb88320 : crc >> 1;
        }
    }

    return crc ^ 0xffffffff;
}

void* Ethernet::Packet::operator new(std::size_t size)
{
    return packetsPool.allocate();
}

void Ethernet::Packet::operator delete(void *ptr)
{
    packetsPool.deallocate(ptr);
}

void Ethernet::Frame::parseBuffer(void)
{
    size_t idx = 0;
    try {
        Memory::consume(_dstMac, _buffer->buf, idx, _bufferSize);
        Memory::consume(_srcMac, _buffer->buf, idx, _bufferSize);
        Memory::consume(_etherType, _buffer->buf, idx, _bufferSize);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ethernet.cpp: Ethernet::Manager<TunDevice>::readDevice: Failed reading ethernet frame header\n";
    }

    _payloadSize = (_etherType < ETHERTYPE_MAX) ? _etherType : _bufferSize - HEADER_SIZE;
    try {
        Memory::consumePointer(_payload, _buffer->buf, idx, _bufferSize, _payloadSize);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ethernet.cpp: Ethernet::Manager<TunDevice>::readDevice: Failed reading ethernet frame payload\n";
    }

    try {
        Memory::consume<CRC32>(_frameCheckSequence, _buffer->buf, idx, _bufferSize);
    } 
    catch (const std::runtime_error& err) {
        std::cerr << "ethernet.cpp: Ethernet::Manager<TunDevice>::readDevice: Failed reading ethernet frame check sequence\n";
    }
}

void Ethernet::Frame::debugPrint(void)
{
    std::cout << "Ethernet Frame: dst: " << _dstMac << " src: " << _srcMac << std::endl;
    std::cout << std::setfill('0') << std::setw(2) << std::hex;
    std::cout << "                ethertype: " << _etherType << " CRC: " << _frameCheckSequence << std::endl;
    std::cout << "                payload addr: " << &_payload; 
    std::cout << std::setfill (' ') << std::setw(0) << std::dec;
    std::cout << " payload size: " << _payloadSize << std::endl;
        
    std::cout << "                Raw Packet:\n";
    std::cout << "                ";
    std::cout << std::setfill('0') << std::setw(2) << std::hex;
    for (size_t i = 0; i < _bufferSize; ++i) {
        std::cout << std::setfill('0') << std::setw(2) << std::hex;
        std::cout << +(uint8_t)(_buffer->buf[i]) << " "; 

        if ((i + 1) % 20 == 0) {
            std::cout << "\n";
            std::cout << "                ";
        }
    }
    std::cout << std::setfill (' ') << std::setw(0) << std::dec << "\n" << std::endl;
}
