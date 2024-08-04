#include "memorypool.hpp"

#include <iostream>

static Memory::ObjectPool<Memory::Block> blocksPool{};

void* Memory::Block::operator new(std::size_t size)
{
    return blocksPool.allocate();
}

void Memory::Block::operator delete(void *ptr)
{
    blocksPool.deallocate(ptr);
}

void Memory::OrderBlocks::markAllocated(std::uintptr_t addr)
{
    std::size_t bit = addr >> (_order + _sizeShift + 1);

    _bitMap[bit] = !_bitMap[bit];
}

bool Memory::OrderBlocks::get(std::uintptr_t addr)
{
    std::size_t bit = addr >> (_order + _sizeShift + 1);
    
    return _bitMap[bit];
}

void Memory::BuddyPool::splitBlock(std::size_t order)
{
    if (order == _totalOrder - 1 && _orderBlocksList[order]._blockList.empty()) {
        throw std::runtime_error("memorypool.cpp: Memory::BuddyPool::splitBlock(): the buddyPool is full");
    }

    if (_orderBlocksList[order]._blockList.empty()) {
        splitBlock(order + 1);
    }

    if (order != 0) { 
        std::unique_ptr<Block> block = std::move(_orderBlocksList[order]._blockList.front());
        _orderBlocksList[order]._blockList.pop_front();
        
        _orderBlocksList[order].markAllocated(block->_addr - _memory.data());
     
        // cutting the block in two smaller halves.
        _orderBlocksList[order - 1].add(block->_addr + (SMALLEST_SIZE << order) / 2);
        _orderBlocksList[order - 1].add(block->_addr);
    } else {
        throw std::runtime_error("memorypool.cpp: Memory::BuddyPool::splitBlock(): can't split order 0 blocks");
    }
}

void Memory::BuddyPool::mergeBlock(unsigned char* addr, std::size_t order)
{
    _orderBlocksList[order].markAllocated(addr - _memory.data());

    if (order != _totalOrder - 1 && !_orderBlocksList[order].get(addr - _memory.data())) {
        unsigned char* high_addr = ((addr - _memory.data()) & ~(1 << (order + SMALLEST_SIZE_SHIFT))) + _memory.data(); 
        unsigned char* low_addr = ((addr - _memory.data()) | (1 << (order + SMALLEST_SIZE_SHIFT))) + _memory.data(); 

        std::list<std::unique_ptr<Block>>& list = _orderBlocksList[order]._blockList;
        for (auto it = list.begin(); it != list.end(); ++it) {
            if (it->get()->_addr == high_addr || it->get()->_addr == low_addr) {
                list.erase(it);
                break;
            }
        }

        mergeBlock(high_addr, order + 1);
    } else {
        _orderBlocksList[order].add(addr);
    }
}

std::size_t Memory::BuddyPool::getOrder(std::size_t size) 
{
    // hack: because 64 >> 6 = 1 but a 64 bit request should be order 0. 
    unsigned int tmp = (size * 2 - 1) >> SMALLEST_SIZE_SHIFT;
    std::size_t sizeOrder = 0;

    while (tmp >>= 1) {
        ++sizeOrder;
    }

    return sizeOrder;
}

void Memory::BuddyPool::deallocate(void *ptr, std::size_t size)
{
    if (ptr == nullptr) {
        throw std::runtime_error("memorypool.cpp: Memory::BuddyPool::deallocate(): ptr is already freed");
    }

    std::size_t sizeOrder = getOrder(size);
    mergeBlock(static_cast<unsigned char*>(ptr), sizeOrder);
    ptr = nullptr;
}

void Memory::consume(IP::Fields1& dest, char *buffer, std::size_t& idx, std::size_t bufferSize)
{
    if (idx + sizeof(uint8_t) > bufferSize) {
        throw std::runtime_error("memorypool.hpp: Memory::consume(): the requested memory would overflow the buffer\n");
    }

    uint8_t val = static_cast<uint8_t>(buffer[idx]);

    dest._version = val >> 4;
    dest._ihl     = val & 0xF; 

    idx += sizeof(uint8_t);
}

void Memory::consume(IP::Fields2& dest, char *buffer, std::size_t& idx, std::size_t bufferSize)
{
    if (idx + sizeof(uint16_t) > bufferSize) {
        throw std::runtime_error("memorypool.hpp: Memory::consume(): the requested memory would overflow the buffer\n");
    }

    uint16_t val = ntohs(*reinterpret_cast<uint16_t*>(buffer + idx));

    dest._flags      = val >> 13;
    dest._fragOffset = val & 0x1FFF; 

    idx += sizeof(uint16_t);
}

void Memory::consume(MacAddr& dest, char *buffer, std::size_t& idx, std::size_t bufferSize)
{
    if (idx + sizeof(uint8_t) * 6 > bufferSize) {
        throw std::runtime_error("memorypool.hpp: Memory::consume(): the requested memory would overflow the buffer\n");
    } 

    for (uint8_t& byte : dest.addr) {
        byte = static_cast<uint8_t>(buffer[idx]);
        idx += sizeof(uint8_t);
    }
}

void Memory::write(const IP::Fields1& src, char *buffer, std::size_t& idx, std::size_t bufferSize)
{
    if (idx + sizeof(uint8_t) > bufferSize) {
        throw std::runtime_error("memorypool.hpp: Memory::write(): writing the memory would overflow the buffer\n");
    }

    uint8_t *ptr = reinterpret_cast<uint8_t*>(buffer + idx);

    *ptr = src._version << 4;
    *ptr |= src._ihl & 0xF; 

    idx += sizeof(uint8_t);
}

void Memory::write(const IP::Fields2& src, char *buffer, std::size_t& idx, std::size_t bufferSize)
{
    if (idx + sizeof(uint16_t) > bufferSize) {
        throw std::runtime_error("memorypool.hpp: Memory::write(): writing the memory would overflow the buffer\n");
    }

    uint16_t *ptr = reinterpret_cast<uint16_t*>(buffer + idx);

    uint16_t val = src._flags << 13;
    val |= src._fragOffset & 0x1FFF;

    *ptr = ntohs(val);

    idx += sizeof(uint16_t);
}

void Memory::write(const MacAddr& src, char *buffer, std::size_t& idx, std::size_t bufferSize)
{
    if (idx + sizeof(uint8_t) * 6 > bufferSize) {
        throw std::runtime_error("memorypool.hpp: Memory::write(): writing the memory would overflow the buffer\n");
    } 

    uint8_t *ptr = reinterpret_cast<uint8_t*>(buffer + idx);
    for (const uint8_t byte : src.addr) {
        *(ptr++) = byte;
        idx += sizeof(uint8_t);
    }
}
