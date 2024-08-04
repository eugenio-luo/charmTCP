#ifndef IPV4_HPP
#define IPV4_HPP

#include "types.hpp"
#include "ethernet.hpp"

namespace IP {
    enum {
        VER_IPV4 = 4,
    };

    enum {
        PRO_ICMP = 1,
        PRO_TCP  = 6,
    };

    enum {
        FLAG_NOFRAG = 0x2,
    };

    enum {
        TYPE_REPLY = 0,
        TYPE_UNREACHABLE = 3,
        TYPE_REQUEST = 8,
    };

    using Protocol = uint8_t;
    using Checksum = uint16_t;
    using TTL      = uint8_t;
    using ID       = uint16_t;
    using Length16 = uint16_t;
    using TOS      = uint8_t;
    using Type     = uint8_t;
    using Code     = uint8_t;
    using Sequence = uint16_t;
    using Length8  = uint8_t;
    using Var      = uint16_t;
 
    constexpr size_t HEADER_SIZE = sizeof(Fields1) + sizeof(TOS) + sizeof(Length16) + sizeof(ID) + 
                                sizeof(Fields2) + sizeof(TTL) + sizeof(Protocol) + sizeof(Checksum) + 2 * sizeof(IPAddr);

    constexpr size_t ICMP_HEADER_SIZE = sizeof(Type) + sizeof(Code) + sizeof(Checksum);

    class Header {
        private:
            char*    _buffer;
            size_t   _payloadSize;

            Fields1  _f1;
            TOS      _tos;
            Length16 _length;
            ID       _id;
            Fields2  _f2;
            TTL      _ttl;
            Protocol _proto;
            Checksum _checksum;
            IPAddr   _srcAddr;
            IPAddr   _dstAddr;
            char*    _payload;

        public:
            size_t getPayloadSize(void) { return _payloadSize; }
            char*  getPayload(void)     { return _payload; }

            size_t readFromBuffer(char *buffer, size_t bufferLength);      
 
            Checksum calculateChecksum(void);

            /* used for TESTS and DEBUG */
        
            void debugPrint(void);

            friend class Manager;
    };

    class PayloadICMPv4Header {
        private:   
            char*    _buffer;
            size_t   _payloadSize;

            Type     _type;
            Code     _code;
            Checksum _checksum;
            char*    _payload;

        public:
            size_t getPayloadSize(void) { return _payloadSize; }
            char*  getPayload(void)     { return _payload; }

            size_t readFromBuffer(char* buffer, size_t bufferLength);

            /* used for TESTS and DEBUG */
        
            void debugPrint(void);

            friend class Manager;
    };

    class PayloadICMPv4Echo {
        private:
            char*    _buffer;
            size_t   _payloadSize;

            ID       _id;
            Sequence _sequence;
            char*    _payload;

        public:
            size_t readFromBuffer(char* buffer, size_t bufferLength);

            /* used for TESTS and DEBUG */
        
            void debugPrint(void);

            friend class Manager;
    };

    class PayloadICMPv4Unreachable {
        private:
            char*    _buffer;
            size_t   _payloadSize;

            uint8_t  _unused;
            Length8  _length;
            Var      _var;
            char*    _payload;

        public:
            size_t readFromBuffer(char* buffer, size_t bufferLength);

            /* used for TESTS and DEBUG */
        
            void debugPrint(void);

            friend class Manager;
    };

    class Manager {
        private:  
            Ethernet::Frame replyMessage(IP::Header& header);
            
            Ethernet::Frame handleICMPRequest(Ethernet::Frame& frame, IP::Header& header, 
                    IP::PayloadICMPv4Header& icmpHeader, IP::PayloadICMPv4Echo& icmpEcho);
            
            Ethernet::Frame handleICMPMessage(Ethernet::Frame& frame, IP::Header& header);

            Checksum calculateChecksum(void *buffer, size_t count);
 
        public:
            Manager() = default;

            Ethernet::Frame handleMessage(Ethernet::Frame& frame);
    };
}

#endif
