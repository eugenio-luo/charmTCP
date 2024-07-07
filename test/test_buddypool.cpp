#include <gtest/gtest.h>

#include "memorypool.hpp"

class BuddyPoolTest : public testing::Test
{
    protected:
        static constexpr std::size_t _size = 512; 
        static constexpr std::size_t _order = 6; 
        Memory::BuddyPool _buddyPool;

        BuddyPoolTest() : _buddyPool{_size, _order} {}
};

TEST_F(BuddyPoolTest, Constructor) 
{
    auto memoryPtr = _buddyPool.memory();
    auto orderBlocksListPtr = _buddyPool.orderBlocksList();
    std::size_t totalMemory = _buddyPool.totalMemory();
    std::size_t totalOrder = _buddyPool.totalOrder();

    ASSERT_EQ(totalOrder, _order);
    ASSERT_EQ(totalMemory, (64 << (_order - 1)) * _size);
    ASSERT_EQ(memoryPtr->size(), totalMemory);
    ASSERT_EQ(orderBlocksListPtr->size(), totalOrder);

    for (std::size_t i = 0; i < totalOrder; ++i) {
        auto& orderBlock = (*orderBlocksListPtr)[i];

        ASSERT_EQ(orderBlock._order, i);
        ASSERT_EQ(orderBlock._bitMap.size(), totalMemory / (128 << i));
    }

    ASSERT_EQ((*orderBlocksListPtr)[totalOrder - 1]._blockList.front()->_addr, memoryPtr->data());
}

TEST_F(BuddyPoolTest, GetOrder)
{
    ASSERT_EQ(_buddyPool.getOrder(48), 0);
    ASSERT_EQ(_buddyPool.getOrder(64), 0);
    ASSERT_EQ(_buddyPool.getOrder(100), 1);
    ASSERT_EQ(_buddyPool.getOrder(128), 1);
    ASSERT_EQ(_buddyPool.getOrder(200), 2);
    ASSERT_EQ(_buddyPool.getOrder(256), 2);
    ASSERT_EQ(_buddyPool.getOrder(400), 3);
    ASSERT_EQ(_buddyPool.getOrder(512), 3);
    ASSERT_EQ(_buddyPool.getOrder(800), 4);
    ASSERT_EQ(_buddyPool.getOrder(1024), 4);
    ASSERT_EQ(_buddyPool.getOrder(2000), 5);
    ASSERT_EQ(_buddyPool.getOrder(2048), 5);
    ASSERT_EQ(_buddyPool.getOrder(4000), 6);
    ASSERT_EQ(_buddyPool.getOrder(4096), 6);
}

TEST_F(BuddyPoolTest, NormalAllocation64b)
{
    unsigned char* buffer = _buddyPool.allocate<unsigned char*>(48);
    auto orderBlocksListPtr = _buddyPool.orderBlocksList();
    auto memoryPtr = _buddyPool.memory();
    std::size_t totalOrder = _buddyPool.totalOrder();

    ASSERT_EQ(buffer, memoryPtr->data());
    ASSERT_EQ((*orderBlocksListPtr)[totalOrder - 1]._blockList.empty(), true);
    ASSERT_EQ((*orderBlocksListPtr)[totalOrder - 1]._bitMap[0], true);

    for (std::size_t i = 0; i < totalOrder - 1; ++i) {
        auto& orderBlocks = (*orderBlocksListPtr)[i];
        unsigned char* ptr = memoryPtr->data() + (64 << i);
        ASSERT_EQ(orderBlocks._blockList.front()->_addr, ptr);
        ASSERT_EQ(orderBlocks._bitMap[0], true);
    }
}

TEST_F(BuddyPoolTest, NormalDeallocation64b)
{
    unsigned char* buffer = _buddyPool.allocate<unsigned char*>(48);
    _buddyPool.deallocate(buffer, 48);

    auto orderBlocksListPtr = _buddyPool.orderBlocksList();
    std::size_t totalOrder = _buddyPool.totalOrder();
    auto memoryPtr = _buddyPool.memory();

    ASSERT_EQ((*orderBlocksListPtr)[totalOrder - 1]._blockList.front()->_addr, buffer);
    ASSERT_EQ((*orderBlocksListPtr)[totalOrder - 1]._bitMap[0], false);
    
    for (std::size_t i = 0; i < totalOrder - 1; ++i) {
        auto& orderBlocks = (*orderBlocksListPtr)[i];
        ASSERT_EQ(orderBlocks._blockList.empty(), true);
        ASSERT_EQ(orderBlocks._bitMap[0], false);
    }
}

TEST_F(BuddyPoolTest, NormalAllocation512b)
{
    unsigned char* buffer = _buddyPool.allocate<unsigned char*>(400);
    auto orderBlocksListPtr = _buddyPool.orderBlocksList();
    auto memoryPtr = _buddyPool.memory();
    std::size_t totalOrder = _buddyPool.totalOrder();

    ASSERT_EQ(buffer, memoryPtr->data());
    ASSERT_EQ((*orderBlocksListPtr)[totalOrder - 1]._blockList.empty(), true);
    ASSERT_EQ((*orderBlocksListPtr)[totalOrder - 1]._bitMap[0], true);

    std::size_t order = _buddyPool.getOrder(400); 
    for (std::size_t i = 0; i < order; ++i) {
        auto& orderBlocks = (*orderBlocksListPtr)[i];
        ASSERT_EQ(orderBlocks._blockList.empty(), true);
        ASSERT_EQ(orderBlocks._bitMap[0], false);
    }

    for (std::size_t i = order; i < totalOrder - 1; ++i) {
        auto& orderBlocks = (*orderBlocksListPtr)[i];
        ASSERT_EQ(orderBlocks._blockList.empty(), false);
        ASSERT_EQ(orderBlocks._bitMap[0], true);
    }
}

TEST_F(BuddyPoolTest, NormalDeallocation512b)
{
    unsigned char* buffer = _buddyPool.allocate<unsigned char*>(400);
    _buddyPool.deallocate(buffer, 400);

    auto orderBlocksListPtr = _buddyPool.orderBlocksList();
    std::size_t totalOrder = _buddyPool.totalOrder();
    auto memoryPtr = _buddyPool.memory();

    ASSERT_EQ((*orderBlocksListPtr)[totalOrder - 1]._blockList.front()->_addr, buffer);
    ASSERT_EQ((*orderBlocksListPtr)[totalOrder - 1]._bitMap[0], false);
    
    for (std::size_t i = 0; i < totalOrder - 1; ++i) {
        auto& orderBlocks = (*orderBlocksListPtr)[i];
        ASSERT_EQ(orderBlocks._blockList.size(), 0);
        ASSERT_EQ(orderBlocks._bitMap[0], false);
    }
}
