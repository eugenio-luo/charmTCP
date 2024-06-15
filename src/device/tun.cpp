#include <system_error>
#include <iostream>
#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_tun.h>

#include "tun.hpp"

TunDevice::TunDevice(const std::optional<std::string_view> dev)
{
    _fd = open(TAP_PATH, O_RDWR);

    if (_fd < 0) {
        throw std::system_error(std::error_code(), "tun.cpp: TunDevice: could not open TUN/TAP device");
    }

    struct ifreq ifr = {};

    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (dev.has_value()) {
        dev.value().copy(ifr.ifr_name, IFNAMSIZ, 0);
    }

    int err = ioctl(_fd, TUNSETIFF, static_cast<void *>(&ifr));

    if (err < 0) {
        close(_fd);
        std::cout << "errno: " << strerror(errno) << '\n';
        throw std::system_error(std::error_code(), "tun.cpp: could not ioctl TUN/TAP device");
    }

    _name = ifr.ifr_name;
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


