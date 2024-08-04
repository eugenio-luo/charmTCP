#include "ip.hpp"
#include "memorypool.hpp"
#include "ethernet.hpp"

IP::Checksum IP::Manager::calculateChecksum(void *buffer, size_t count)
{
    uint32_t sum = 0;
    uint16_t *ptr = reinterpret_cast<uint16_t*>(buffer);

    while (count > 1) {
        sum += *ptr++;
        count -= 2;
    }

    if (count > 0) {
        sum += *reinterpret_cast<uint8_t*>(ptr);
    }

    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return ~sum;
}

size_t IP::Header::readFromBuffer(char *buffer, size_t bufferLength)
{
    _buffer = buffer;

    size_t idx = 0;
    try {
        Memory::consume(_f1, buffer, idx, bufferLength); 
        Memory::consume(_tos, buffer, idx, bufferLength); 
        Memory::consume(_length, buffer, idx, bufferLength); 
        Memory::consume(_id, buffer, idx, bufferLength); 
        Memory::consume(_f2, buffer, idx, bufferLength); 
        Memory::consume(_ttl, buffer, idx, bufferLength); 
        Memory::consume(_proto, buffer, idx, bufferLength); 
        Memory::consume(_checksum, buffer, idx, bufferLength); 
        Memory::consume(_srcAddr, buffer, idx, bufferLength); 
        Memory::consume(_dstAddr, buffer, idx, bufferLength); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::Header::readFromBuffer: Failed reading IP header\n";
    }

    _payloadSize = bufferLength - idx;
    try {
        Memory::consumePointer(_payload, buffer, idx, bufferLength, _payloadSize); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::Header::readFromBuffer: Failed reading IP payload\n";
    }

    return _payloadSize;
}

void IP::Header::debugPrint(void)
{
    std::cout << std::setfill('0') << std::setw(2) << std::hex;
    std::cout << "IP Header: version: " << +_f1._version << " ihl: " << +_f1._ihl << std::endl;
    std::cout << "           tos: " << +_tos << " length: " << +_length << std::endl;
    std::cout << "           id: " << +_id << " flags: " << +_f2._flags << std::endl;
    std::cout << "           frag offset: " << +_f2._fragOffset << " ttl: " << +_ttl << std::endl;
    std::cout << "           protocol: " << +_proto << " checksum: " << +_checksum << std::endl;
    std::cout << "           src IP: " << +_srcAddr << " dst IP: " << +_dstAddr << std::endl;
    std::cout << "           payload addr: " << &_payload << " payload size: " << +_payloadSize << std::endl; 

    std::cout << "           Raw Packet:\n";
    std::cout << "           ";
    for (int i = 0; i < 20; ++i) {
        std::cout << std::setfill('0') << std::setw(2) << std::hex;
        std::cout << +(uint8_t)(_buffer[i]) << " "; 
    }
    std::cout << std::setfill (' ') << std::setw(0) << std::dec << "\n" << std::endl;
}

size_t IP::PayloadICMPv4Header::readFromBuffer(char *buffer, size_t bufferLength)
{
    _buffer = buffer;

    size_t idx = 0;
    try {
        Memory::consume(_type, buffer, idx, bufferLength); 
        Memory::consume(_code, buffer, idx, bufferLength); 
        Memory::consume(_checksum, buffer, idx, bufferLength); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::PayloadICMPv4::readFromBuffer: Failed reading ICMPv4 header\n";
    }

    _payloadSize = bufferLength - idx;
    try {
        Memory::consumePointer(_payload, buffer, idx, bufferLength, _payloadSize); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::PayloadICMPv4::readFromBuffer: Failed reading ICMPv4 payload\n";
    }

    return _payloadSize;
}

void IP::PayloadICMPv4Header::debugPrint(void)
{
    std::cout << std::setfill('0') << std::setw(2) << std::hex;
    std::cout << "ICMPv4 Header: type: " << +_type << " code: " << +_code << std::endl;
    std::cout << "               checksum: " << +_checksum << std::endl;
    std::cout << "               payload addr: " << &_payload << " payload size: " << +_payloadSize << std::endl; 

    std::cout << "               Raw Packet:\n";
    std::cout << "               ";
    for (int i = 0; i < 10; ++i) {
        std::cout << std::setfill('0') << std::setw(2) << std::hex;
        std::cout << +(uint8_t)(_buffer[i]) << " "; 
    }
    std::cout << std::setfill (' ') << std::setw(0) << std::dec << "\n" << std::endl;
}

size_t IP::PayloadICMPv4Echo::readFromBuffer(char *buffer, size_t bufferLength)
{
    _buffer = buffer;

    size_t idx = 0;
    try {
        Memory::consume(_id, buffer, idx, bufferLength); 
        Memory::consume(_sequence, buffer, idx, bufferLength); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::PayloadICMPv4Echo::readFromBuffer: Failed reading ICMPv4 echo header\n";
    }

    _payloadSize = bufferLength - idx;
    std::cout << "DATA SIZE: " << _payloadSize << '\n';
    try {
        Memory::consumePointer(_payload, buffer, idx, bufferLength, _payloadSize); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::PayloadICMPv4Echo::readFromBuffer: Failed reading ICMPv4 echo payload\n";
    }

    return _payloadSize;
}

void IP::PayloadICMPv4Echo::debugPrint(void)
{
    std::cout << std::setfill('0') << std::setw(2) << std::hex;
    std::cout << "ICMPv4 Echo Header: id: " << +_id << " sequence: " << +_sequence << std::endl;
    std::cout << "                    payload addr: " << &_payload << " payload size: " << +_payloadSize << std::endl; 

    std::cout << "                    Raw Packet:\n";
    std::cout << "                    ";
    for (int i = 0; i < 5; ++i) {
        std::cout << std::setfill('0') << std::setw(2) << std::hex;
        std::cout << +(uint8_t)(_buffer[i]) << " "; 
    }
    std::cout << std::setfill (' ') << std::setw(0) << std::dec << "\n" << std::endl;
}

size_t IP::PayloadICMPv4Unreachable::readFromBuffer(char *buffer, size_t bufferLength)
{
    _buffer = buffer;

    size_t idx = 0;
    try {
        Memory::consume(_unused, buffer, idx, bufferLength); 
        Memory::consume(_length, buffer, idx, bufferLength); 
        Memory::consume(_var, buffer, idx, bufferLength); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::PayloadICMPv4Unreachable::readFromBuffer: Failed reading ICMPv4 unreachable header\n";
    }

    _payloadSize = bufferLength - idx;
    try {
        Memory::consumePointer(_payload, buffer, idx, bufferLength, _payloadSize); 
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::PayloadICMPv4Unreachable::readFromBuffer: Failed reading ICMPv4 unreachable payload\n";
    }

    return _payloadSize;
}

void IP::PayloadICMPv4Unreachable::debugPrint(void)
{
    std::cout << std::setfill('0') << std::setw(2) << std::hex;
    std::cout << "ICMPv4 Unreachable Header: length: " << +_length << " var: " << +_var << std::endl;
    std::cout << "                           payload addr: " << &_payload << " payload size: " << +_payloadSize << std::endl; 

    std::cout << "                    Raw Packet:\n";
    std::cout << "                    ";
    for (int i = 0; i < 5; ++i) {
        std::cout << std::setfill('0') << std::setw(2) << std::hex;
        std::cout << +(uint8_t)(_buffer[i]) << " "; 
    }
    std::cout << std::setfill (' ') << std::setw(0) << std::dec << "\n" << std::endl;
}

Ethernet::Frame IP::Manager::handleICMPRequest(Ethernet::Frame& frame, IP::Header& header, 
                                            IP::PayloadICMPv4Header& icmpHeader, IP::PayloadICMPv4Echo& icmpEcho)
{
    static uint32_t id_num = 1;

    Ethernet::Frame reply;
    size_t bufferLength = reply.allocPacket();
    Ethernet::Packet *buffer = reply.getPacket();
    MacAddr addr = getDevMacAddr();
    
    size_t idx = 0;
    try {
        Memory::write(frame.getSrc(), buffer->buf, idx, bufferLength);
        Memory::write(addr, buffer->buf, idx, bufferLength);
        Memory::write(static_cast<EtherType>(PRO_IPV4), buffer->buf, idx, bufferLength);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::Manager::handleICMPRequest: Failed writing ethernet frame header\n";
    }

    size_t ipStart = idx;
    size_t ipLenStart; 
    size_t ipChecksumStart;
    try {
        Memory::write(IP::Fields1(VER_IPV4, 5), buffer->buf, idx, bufferLength);
        Memory::write(header._tos, buffer->buf, idx, bufferLength);
        ipLenStart = idx;
        Memory::write(static_cast<Length16>(0), buffer->buf, idx, bufferLength);
        Memory::write(static_cast<ID>(id_num++), buffer->buf, idx, bufferLength);
        Memory::write(IP::Fields2(0, 0), buffer->buf, idx, bufferLength);
        Memory::write(static_cast<TTL>(64), buffer->buf, idx, bufferLength);
        Memory::write(static_cast<Protocol>(IP::PRO_ICMP), buffer->buf, idx, bufferLength);
        ipChecksumStart = idx;
        Memory::write(static_cast<Checksum>(0), buffer->buf, idx, bufferLength);
        Memory::write(header._dstAddr, buffer->buf, idx, bufferLength);
        Memory::write(header._srcAddr, buffer->buf, idx, bufferLength);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::Manager::handleICMPRequest: Failed writing IP header\n";
    }

    size_t icmpStart = idx;
    size_t icmpChecksumStart;
    try {
        Memory::write(static_cast<Type>(TYPE_REPLY), buffer->buf, idx, bufferLength);
        Memory::write(static_cast<Code>(0), buffer->buf, idx, bufferLength);
        icmpChecksumStart = idx;
        Memory::write(static_cast<Checksum>(0), buffer->buf, idx, bufferLength);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::Manager::handleICMPRequest: Failed writing ICMP header\n";
    }

    try {
        Memory::write(icmpEcho._id, buffer->buf, idx, bufferLength);
        Memory::write(icmpEcho._sequence, buffer->buf, idx, bufferLength);
        Memory::writePointer(icmpEcho._payload, buffer->buf, idx, bufferLength, icmpEcho._payloadSize);
        Memory::write(frame.getCRC(), buffer->buf, idx, bufferLength);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::Manager::handleICMPRequest: Failed writing ICMP reply\n";
    }

    try {
        Memory::write(static_cast<Length16>(idx - ipStart), buffer->buf, ipLenStart, bufferLength);
        
        Checksum icmpChecksum = htons(calculateChecksum(buffer->buf + icmpStart, idx - icmpStart));
        Memory::write(static_cast<Checksum>(icmpChecksum), buffer->buf, icmpChecksumStart, bufferLength);

        Checksum checksum = htons(calculateChecksum(buffer->buf + ipStart, idx - ipStart));
        Memory::write(static_cast<Checksum>(checksum), buffer->buf, ipChecksumStart, bufferLength);

        CRC32 newCRC = htonl(Ethernet::calcCRC(0, buffer->buf, idx));
        Memory::write(newCRC, buffer->buf, idx, bufferLength);
    }
    catch (const std::runtime_error& err) {
        std::cerr << "ip.cpp: IP::Manager::handleICMPRequest: Failed writing checksums\n";
    }

    reply.setBufferSize(idx);
    reply.parseBuffer();
    std::cout << "RESPONSE:\n";
    reply.debugPrint();
    return reply;
}

Ethernet::Frame IP::Manager::handleICMPMessage(Ethernet::Frame& frame, IP::Header& header)
{
    char *buffer = header.getPayload();   
    size_t bufferLength = header.getPayloadSize();

    IP::PayloadICMPv4Header icmpHeader;
    icmpHeader.readFromBuffer(buffer, bufferLength);
    icmpHeader.debugPrint();


    switch (icmpHeader._type) {
        case TYPE_REQUEST: {
            buffer = icmpHeader.getPayload();
            bufferLength = icmpHeader.getPayloadSize();

            IP::PayloadICMPv4Echo icmpEcho;
            icmpEcho.readFromBuffer(buffer, bufferLength);
            return handleICMPRequest(frame, header, icmpHeader, icmpEcho);
        }
            
        case TYPE_UNREACHABLE:
            throw std::runtime_error("ip.cpp: IP::Manager::HandleICMPMessage(): unreachable\n");
        
        default:
            throw std::runtime_error("ip.cpp: IP::Manager::HandleICMPMessage(): not supported version type\n");
    }
}

Ethernet::Frame IP::Manager::handleMessage(Ethernet::Frame& frame)
{
    char *buffer = frame.getPayload();
    size_t bufferLength = frame.getPayloadSize();

    IP::Header header;
    header.readFromBuffer(buffer, bufferLength);   
    header.debugPrint();

    if (header._f1._version != VER_IPV4) {
        throw std::runtime_error("ip.cpp: IP::Manager::HandleMessage(): not supported version type\n");
    }
    
    if (header._f1._ihl < 5) {
        throw std::runtime_error("ip.cpp: IP::Manager::HandleMessage(): IPv4 header length must be at least 5\n");
    }

    if (header._ttl == 0) {
        throw std::runtime_error("ip.cpp: IP::Manager::HandleMessage(): Time to live == 0\n");
    }

    Checksum checksum = calculateChecksum(header._buffer, IP::HEADER_SIZE);

    if (checksum != 0) {
        throw std::runtime_error("ip.cpp: IP::Manager::HandleMessage(): checksum not correct\n");
    }

    switch (header._proto) {
        case PRO_ICMP:
            return handleICMPMessage(frame, header);        
            break;

        default:
            throw std::runtime_error("ip.cpp: IP::Manager::HandleMessage(): not supported protocol type\n");
            break;
    }

    return {};
}
