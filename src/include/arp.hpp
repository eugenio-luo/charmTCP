#ifndef ARP_HPP
#define ARP_HPP

#include <unordered_map>

#include "types.hpp"
#include "ethernet.hpp"

namespace ARP
{
    enum {
        OP_REQUEST = 1,
        OP_REPLY = 2
    };

    using Size   = uint8_t;
    using OpCode = uint16_t;

    class Header 
    {
        private:
            char*   _buffer;
            size_t  _payloadSize;

            HwType  _hwType;
            ProType _proType;
            Size    _hwSize;
            Size    _proSize;
            OpCode  _opCode;
            char*   _payload;

        public:
            /* return the payload size */
            size_t readFromBuffer(char *buffer, size_t bufferLength);

            char* getPayload(void) { return _payload; }

            /* used for TESTS and DEBUG */
        
            void debugPrint(void);
 
            friend class CacheManager;
    };

    class PayloadIPv4 
    {
        private:
            char*   _buffer;
            
            MacAddr _srcMac;
            IPAddr  _srcIP;
            MacAddr _dstMac;
            IPAddr  _dstIP;

        public:
            void readFromBuffer(char *buffer, size_t bufferLength);
   
            /* used for TESTS and DEBUG */
       
            void debugPrint(void);
            
            friend class CacheManager;
    };

    struct Cache
    {
        HwType  _hwType;
        IPAddr  _ipAddr;
        MacAddr _macAddr;
        int     _state = false;
    };

    class CacheManager
    {
        private:
            std::unordered_map<IPAddr, Cache>    _cacheEntries;

            Ethernet::Frame replyMessage(ARP::Header& header, ARP::PayloadIPv4& data, CRC32 oldCRC);
        
        public:
            Ethernet::Frame handleMessage(Ethernet::Frame& frame);
            /* used for TESTS and DEBUG */
            
            void debugPrint(void);
    };
}

#endif
