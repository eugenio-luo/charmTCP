#include <system_error>
#include <iostream>
#include <cerrno>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_tun.h>

#include "tun.hpp"

static MacAddr deviceMacAddr{};

MacAddr getDevMacAddr(void)
{
    return deviceMacAddr;
}

MacAddr TunDevice::getMacAddr(void)
{
    MacAddr ret{};
    struct ifreq ifr{};
    strcpy(ifr.ifr_name, _name.c_str());
    if (ioctl(_fd, SIOCGIFHWADDR, &ifr) == 0) {
        for (int i =0; i < 6; ++i) {
            ret.addr[i] =  static_cast<uint8_t>(ifr.ifr_addr.sa_data[i]);
        }
    }
    
    deviceMacAddr = ret;
    return ret;
}

TunDevice::TunDevice(const std::optional<std::string_view> dev)
{
    _fd = open(TAP_PATH, O_RDWR);

    if (_fd < 0) {
        throw std::system_error(std::error_code(), "tun.cpp: TunDevice(): could not open TUN/TAP device");
    }

    struct ifreq ifr{};
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (dev.has_value()) {
        dev.value().copy(ifr.ifr_name, IFNAMSIZ, 0);
    }

    int err = ioctl(_fd, TUNSETIFF, static_cast<void *>(&ifr));

    if (err < 0) {
        close(_fd);
        std::cout << "errno: " << strerror(errno) << '\n';
        throw std::system_error(std::error_code(), "tun.cpp: TunDevice(): could not ioctl TUN/TAP device");
    }

    _name = ifr.ifr_name;
    _addr = getMacAddr();
}

TunDevice::~TunDevice() 
{
    if (_fd != -1) {
        close(_fd);
    }
}

TunDevice::TunDevice(TunDevice&& other)
{
    _fd = other._fd;
    _name = std::move(other._name);
    other._fd = -1;
}

TunDevice& TunDevice::operator=(TunDevice&& other)
{
    close(_fd);

    _fd = other._fd;
    _name = std::move(other._name);
    other._fd = -1;
    return *this;
}

int TunDevice::readBuf(char* buf, size_t count)
{
    return read(_fd, buf, count);
}

int TunDevice::writeBuf(char* buf, size_t count)
{
    return write(_fd, buf, count);
}
