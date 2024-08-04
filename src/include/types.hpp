#ifndef TYPES_HPP 
#define TYPES_HPP

#include <iostream>
#include <iomanip>

struct MacAddr
{
    std::array<uint8_t, 6> addr = {0, 0, 0, 0, 0, 0};
};

inline std::ostream &operator<<(std::ostream &os, MacAddr const &macAddr) {
    for (uint8_t byte : macAddr.addr) {
        os << std::setfill('0') << std::setw(2) << std::hex;
        os << +byte << ' ';
    }
    return os << std::setfill (' ') << std::setw(0) << std::dec;
}

using CRC32     = uint32_t;
using EtherType = uint16_t;
using HwType    = uint16_t;
using ProType   = uint16_t;
using IPAddr    = uint32_t;

enum {
    HW_RESERVED = 0,
    HW_ETHERNET = 1,
};

enum {
    PRO_IPV4 = 0x800,
    PRO_ARP  = 0x806,
};

namespace IP {
    struct Fields1 {
        uint8_t  _version   : 4;
        uint8_t  _ihl       : 4;
 
        Fields1() = default;

        Fields1(uint8_t version, uint8_t ihl) : _version{version}, _ihl{ihl} {}
    };

    struct Fields2 {
        uint16_t _flags      : 3; 
        uint16_t _fragOffset : 13;
        
        Fields2() = default;
        
        Fields2(uint16_t flags, uint16_t fragOffset) : _flags{flags}, _fragOffset{fragOffset} {}
    };
}

#endif
