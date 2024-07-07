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