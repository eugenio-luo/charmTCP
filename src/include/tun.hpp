#ifndef DEVICE_TUN_HPP
#define DEVICE_TUN_HPP

#include <optional>
#include <string_view>
#include <memory>

#include "types.hpp"

MacAddr getDevMacAddr(void); 

class TunDevice
{
    private:
        static constexpr char TAP_PATH[] = "/dev/net/tun";
            
        int         _fd = -1;
        std::string _name;
        MacAddr     _addr;

        MacAddr getMacAddr();

    public:

        static constexpr char TUN_NAME[] = "tun%d";

        TunDevice(const std::optional<std::string_view> dev);

        ~TunDevice();
 
        TunDevice() = delete;

        TunDevice(const TunDevice&) = delete;

        TunDevice& operator=(const TunDevice&) = delete;

        TunDevice(TunDevice&& other);

        TunDevice& operator=(TunDevice&& other);

        std::string name() const { return _name; }
        MacAddr     addr() const { return _addr; }
        int         fd()   const { return _fd;   }
    
        int readBuf(char* buf, size_t count);

        int writeBuf(char* buf, size_t count);
};

#endif
