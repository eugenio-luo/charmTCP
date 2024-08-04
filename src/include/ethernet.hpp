#ifndef ETHERNET_HPP
#define ETHERNET_HPP

#include <linux/if_ether.h>

#include "memorypool.hpp"
#include "types.hpp"
#include "tun.hpp"

namespace Ethernet
{
    constexpr size_t ETHERTYPE_MAX  = 0x600;
    constexpr size_t MTU            = 1500; 
    constexpr size_t HEADER_SIZE    = sizeof(char[6]) * 2 + sizeof(EtherType) + sizeof(CRC32);
    constexpr size_t MAX_FRAME_SIZE = MTU + HEADER_SIZE; 
    constexpr size_t MIN_FRAME_SIZE = 64;

    CRC32 calcCRC(CRC32 crc, void *buffer, size_t bufferLength); 

    struct Packet 
    {
        char buf[MAX_FRAME_SIZE];
        
        static void* operator new(std::size_t size);   
        static void operator delete(void *ptr);
    };
 
    template <typename T>
    class Manager;

    class Frame
    {
        private:
            std::unique_ptr<Packet> _buffer;
            size_t                  _bufferSize;
            size_t                  _payloadSize;

            MacAddr                 _dstMac;
            MacAddr                 _srcMac;
            EtherType               _etherType;
            char*                   _payload;
            CRC32                   _frameCheckSequence;    

        public:
            MacAddr   getDst(void)         { return _dstMac; }
            MacAddr   getSrc(void)         { return _srcMac; }
            EtherType getType(void)        { return _etherType; }
            char*     getPayload(void)     { return _payload; }
            size_t    getPayloadSize(void) { return _payloadSize; }
            Packet*   getPacket(void)      { return _buffer.get(); }
            CRC32     getCRC(void)         { return _frameCheckSequence; }

            void      setBufferSize(size_t size) { _bufferSize = size; }

            size_t    allocPacket(void)    
            { 
                _buffer = std::unique_ptr<Ethernet::Packet>(new Ethernet::Packet()); 
                return MAX_FRAME_SIZE; 
            }

            void       parseBuffer(void);

            /* used for TESTS and DEBUG */

            void debugPrint(void);
 
            template <typename T>
            friend class Ethernet::Manager;
    };

    template <typename T>
    class Manager 
    {
        private:
            T _device;
        
        public:
    };

    template<>
    class Manager<TunDevice>
    {
        private:
            TunDevice _device;
        
        public:
            Manager(const char *name) : _device{name} {}

            Ethernet::Frame readDevice(void)
            {
                Ethernet::Frame frame;
                frame._buffer = std::unique_ptr<Ethernet::Packet>(new Ethernet::Packet());
                frame._bufferSize = _device.readBuf(frame._buffer->buf, MAX_FRAME_SIZE);

                frame.parseBuffer();
                    
                return frame;
            }

            void writeDevice(Ethernet::Frame& frame) 
            {
                _device.writeBuf(frame._buffer->buf, frame._bufferSize);
            }
    };
}
#endif
