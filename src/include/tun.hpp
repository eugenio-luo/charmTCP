#ifndef DEVICE_TUN_HPP
#define DEVICE_TUN_HPP

#include <optional>
#include <string_view>
#include <memory>

class TunDevice
{
    private:
        static constexpr char TAP_PATH[] = "/dev/net/tun";
            
        int _fd = -1;
        std::string _name{};

    public:

        static constexpr char TUN_NAME[] = "tun%d";

        TunDevice(const std::optional<std::string_view> dev);

        ~TunDevice();
 
        TunDevice() = delete;

        TunDevice(const TunDevice&) = delete;

        TunDevice& operator=(const TunDevice&) = delete;

        TunDevice(TunDevice&& other);

        TunDevice& operator=(TunDevice&& other);

        std::string name() const { return _name; };
        
        int fd() const {return _fd; };
};

#endif
