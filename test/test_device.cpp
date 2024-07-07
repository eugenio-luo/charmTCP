#include <gtest/gtest.h>

#include "tun.hpp"

TEST(TunDeviceTest, TunDeviceCreation) 
{
    
    constexpr char name[] = "TESTNAME";

    TunDevice tunDev{name};

    EXPECT_STREQ(tunDev.name().c_str(), name) << "TUN/TAP device's name is different from the one requested.";
    ASSERT_GT(tunDev.fd(), -1) << "TUN/TAP device failed to obtain a file descriptor.";
}
