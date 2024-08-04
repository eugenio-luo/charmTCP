#include "arp.hpp"
#include "memorypool.hpp"
#include "ethernet.hpp"

Ethernet::Frame ARP::CacheManager::handleMessage(Ethernet::Frame& frame)
{
    char *buffer = frame.getPayload();
    size_t bufferLength = frame.getPayloadSize();

    ARP::Header header;
    header.readFromBuffer(buffer, bufferLength);   
    header.debugPrint();

    if (header._hwType != HW_ETHERNET) {
        throw std::runtime_error("arp.cpp: ARP::CacheManager::HandleMessage(): not supported hardware type\n");
    }

    if (header._proType != PRO_IPV4) {
        throw std::runtime_error("arp.cpp: ARP::CacheManager::HandleMessage(): not supported protocol type\n");
    }

    PayloadIPv4 data;
    data.readFromBuffer(header._payload, header._payloadSize);
    data.debugPrint();   

    Cache& entry = _cacheEntries[data._srcIP];
    if (!entry._state) {
    
        entry._hwType = header._hwType;
        entry._ipAddr = data._srcIP;
        entry._macAddr = data._srcMac;
        entry._state = true;

    } else {

        if (entry._hwType == header._hwType) {
            entry._macAddr = data._srcMac;
        }
    }

    if (header._opCode == OP_REQUEST) {
        
        return replyMessage(header, data, frame.getCRC());
    
    } else {
        throw std::runtime_error("arp.cpp: ARP::CacheManager::HandleMessage(): not supported opcode\n");
    }
}

Ethernet::Frame ARP::CacheManager::replyMessage(ARP::Header& header, ARP::PayloadIPv4& data, CRC32 oldCRC)
{
    Ethernet::Frame frame{};
    size_t bufferLength = frame.allocPacket();
    Ethernet::Packet *buffer = frame.getPacket();
    MacAddr addr = getDevMacAddr();

    size_t idx = 0;
    try {
        Memory::write(data._srcMac, buffer->buf, idx, bufferLength);
        Memory::write(addr, buffer->buf, idx, bufferLength);
        Memory::write(static_cast<EtherType>(PRO_ARP), buffer->buf, idx, bufferLength);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "arp.cpp: ARP::CacheManager::replyMessage: Failed writing ethernet frame header\n";
    }

    try {
        Memory::write(header._hwType, buffer->buf, idx, bufferLength);
        Memory::write(header._proType, buffer->buf, idx, bufferLength);
        Memory::write(header._hwSize, buffer->buf, idx, bufferLength);
        Memory::write(header._proSize, buffer->buf, idx, bufferLength);
        Memory::write(static_cast<ARP::OpCode>(OP_REPLY), buffer->buf, idx, bufferLength);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "arp.cpp: ARP::CacheManager::replyMessage: Failed writing ARP header\n";
    }

    try {
        Memory::write(addr, buffer->buf, idx, bufferLength);
        Memory::write(data._dstIP, buffer->buf, idx, bufferLength);
        Memory::write(data._srcMac, buffer->buf, idx, bufferLength);
        Memory::write(data._srcIP, buffer->buf, idx, bufferLength);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "arp.cpp: ARP::CacheManager::replyMessage: Failed writing ARP data\n";
    }

    while (idx < Ethernet::MIN_FRAME_SIZE - sizeof(CRC32)) {
        Memory::write(static_cast<char>(0), buffer->buf, idx, bufferLength);
    }

    CRC32 newCRC = htonl(Ethernet::calcCRC(oldCRC, buffer->buf, idx));
    try {
        Memory::write(newCRC, buffer->buf, idx, bufferLength);
    } 
    catch (const std::runtime_error& err) {
        std::cerr << "arp.cpp: ARP::CacheManager::replyMessage: Failed writing ethernet frame check sequence\n";
    }

    frame.setBufferSize(idx);
    frame.parseBuffer();
    frame.debugPrint();
    return frame;
}

void ARP::CacheManager::debugPrint(void)
{
    std::cout << "ARP cache entries:" << std::endl;

    for (auto& pair : _cacheEntries) {

        Cache& entry = pair.second;
        std::cout << "    hwtype: " << entry._hwType << " IP addr: " 
            << entry._ipAddr << " MAC addr: " << entry._macAddr << std::endl;
    }
    std::cout << std::endl;
}

size_t ARP::Header::readFromBuffer(char* buffer, size_t bufferLength)
{
    _buffer = buffer;

    size_t idx = 0;
    try {
        Memory::consume(_hwType, buffer, idx, bufferLength); 
        Memory::consume(_proType, buffer, idx, bufferLength); 
        Memory::consume(_hwSize, buffer, idx, bufferLength); 
        Memory::consume(_proSize, buffer, idx, bufferLength); 
        Memory::consume(_opCode, buffer, idx, bufferLength); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "arp.cpp: ARP::Header::readFromBuffer: Failed reading ARP header\n";
    }

    _payloadSize = bufferLength - idx;
    try {
        Memory::consumePointer(_payload, buffer, idx, bufferLength, _payloadSize); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "arp.cpp: ARP::Header::readFromBuffer: Failed reading ARP payload\n";
    }

    return _payloadSize;
}

void ARP::Header::debugPrint(void)
{
    std::cout << std::setfill('0') << std::setw(2) << std::hex;
    std::cout << "ARP Header: hwtype: " << _hwType << " protype: " << _proType << std::endl;
    std::cout << "            hwsize: " << +_hwSize << " prosize: " << +_proSize << std::endl;
    std::cout << "            opcode: " << _opCode << std::endl;
    std::cout << "            payload addr: " << &_payload << " payload size: " << _payloadSize << std::endl; 

    std::cout << "            Raw Packet:\n";
    std::cout << "            ";
    for (int i = 0; i < 20; ++i) {
        std::cout << std::setfill('0') << std::setw(2) << std::hex;
        std::cout << +(uint8_t)(_buffer[i]) << " "; 
    }
    std::cout << std::setfill (' ') << std::setw(0) << std::dec << "\n" << std::endl;
}

void ARP::PayloadIPv4::readFromBuffer(char *buffer, size_t bufferLength)
{
    _buffer = buffer;

    size_t idx = 0;
    try {
        Memory::consume(_srcMac, buffer, idx, bufferLength); 
        Memory::consume(_srcIP, buffer, idx, bufferLength); 
        Memory::consume(_dstMac, buffer, idx, bufferLength); 
        Memory::consume(_dstIP, buffer, idx, bufferLength); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "arp.cpp: ARP::PayloadIPv4::readFromBuffer: Failed reading ARP IPv4 Payload\n";
    }
}

void ARP::PayloadIPv4::debugPrint(void)
{
    std::cout << "ARP IPv4 Payload: src MAC: " << _srcMac << " src IP: " << _srcIP << std::endl;
    std::cout << "                  dst MAC: " << _dstMac << " dst IP: " << _dstIP << '\n' << std::endl;

    std::cout << "            Raw Packet:\n";
    std::cout << "            ";
    for (size_t i = 0; i < sizeof(uint8_t) * 12 + sizeof(IPAddr) * 2; ++i) {
        std::cout << std::setfill('0') << std::setw(2) << std::hex;
        std::cout << +(uint8_t)(_buffer[i]) << " "; 
    }
    std::cout << std::setfill (' ') << std::setw(0) << std::dec << "\n" << std::endl;
}
