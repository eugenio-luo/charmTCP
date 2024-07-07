#ifndef MEMORYPOOL_HPP
#define MEMORYPOOL_HPP

#include <cstdint>
#include <stack>
#include <vector>
#include <list>
#include <memory>
#include <stdexcept>

namespace Memory 
{
    template <typename T>
    class ObjectPool 
    {
        private:
    static constexpr std::size_t DEFAULT_TOTAL_BLOCKS = 1000;
    
            std::vector<T>     _memory;
            std::size_t        _totalBlocks;
            std::stack<std::size_t> _freeBlocks;
        
        public:
            ObjectPool(std::size_t totalBlocks = DEFAULT_TOTAL_BLOCKS)
                : _memory(totalBlocks),
                  _totalBlocks{totalBlocks},
                  _freeBlocks{}
            {
                for (std::size_t i = 0; i < totalBlocks; ++i) {
                    _freeBlocks.push(i);
                }
            }
  
            T* allocate()
            {
                if (_freeBlocks.empty()) {
                    throw std::runtime_error("memorypool.cpp: Memory::ObjectPool<T>::allocate(): there aren't any free blocks");
                }
            
                std::size_t blockIndex = _freeBlocks.top();
                _freeBlocks.pop();
            
                return &(_memory[blockIndex]);
            }
            
            void deallocate(void* ptr)
            {
                T* block = static_cast<T*>(ptr); 
                if (block == nullptr) {
                    throw std::runtime_error("memorypool.cpp: Memory::ObjectPool<T>::deallocate(): block is nullptr");
                }
            
                std::size_t blockIndex = block - &(_memory[0]);
                _freeBlocks.push(blockIndex);
            }

            /* used for TESTS and DEBUG */

            std::vector<T>* memory() { return &_memory; }
 
            std::stack<std::size_t>* freeBlocks() { return &_freeBlocks; }
    };
 
    class Block 
    {
        public:
            unsigned char* _addr;
        
        Block() = default;

        Block(unsigned char* addr) : _addr{addr} {} 

        static void* operator new(std::size_t size);
        static void operator delete(void *ptr);
    };
 
    class OrderBlocks
    {
        public:
            std::list<std::unique_ptr<Block>> _blockList;
            std::vector<bool>                 _bitMap;
            std::size_t                       _order;
            std::size_t                       _sizeShift;

        void markAllocated(std::uintptr_t addr);
    
        bool get(std::uintptr_t addr);
        
        void add(unsigned char* addr) 
        {
            _blockList.push_front(std::make_unique<Block>(addr));
        }
    };

    class BuddyPool
    {
        private:
            // lists for 64, 128, 256, 512, 1024, 2048 bytes
            // order 0 is 64, order N is (64 << N)
            static constexpr std::size_t SMALLEST_SIZE = 64;
            static constexpr std::size_t SMALLEST_SIZE_SHIFT = 6;
            static constexpr std::size_t DEFAULT_ORDER = 6;
            static constexpr std::size_t DEFAULT_SIZE = 512;

            std::size_t                    _totalMemory;
            std::size_t                    _totalOrder;
            std::vector<unsigned char>     _memory;
            std::vector<OrderBlocks>       _orderBlocksList;

            void splitBlock(std::size_t order);
    
            void mergeBlock(unsigned char* addr, std::size_t order);

        public:
            BuddyPool(std::size_t size = DEFAULT_SIZE, std::size_t order = DEFAULT_ORDER) 
                : _totalMemory{size * (SMALLEST_SIZE << (order - 1))},
                  _totalOrder{order},
                  _memory(_totalMemory),
                  _orderBlocksList(order) 
            {
                std::size_t nbits = _totalMemory / 128;
                std::size_t i = 0;
                for (auto& orderBlocks : _orderBlocksList) {
                    orderBlocks._bitMap = std::vector<bool>(nbits);
                    orderBlocks._order = i++;    
                    orderBlocks._sizeShift = SMALLEST_SIZE_SHIFT;

                    nbits >>= 1;
                }

                unsigned char* addr = static_cast<unsigned char*>(&_memory[0]);
                _orderBlocksList[order - 1].add(addr);
            }

            static std::size_t getOrder(std::size_t size);

            template <typename T>
            T allocate(std::size_t size)
            {
                static_assert(std::is_pointer<T>::value, "Expected pointer");

                if (size > _totalMemory) {
                throw std::runtime_error("memorypool.cpp: Memory::BuddyPool::allocate(): asking for more memory than available");
    }

                std::size_t sizeOrder = getOrder(size);
    
                if (_orderBlocksList[sizeOrder]._blockList.empty()) {
                    splitBlock(sizeOrder + 1);  
                }

                std::unique_ptr<Block> block = std::move(_orderBlocksList[sizeOrder]._blockList.front());
                _orderBlocksList[sizeOrder]._blockList.pop_front();
    
                _orderBlocksList[sizeOrder].markAllocated(block->_addr - _memory.data());
    
                return block->_addr;
            }
            
            void deallocate(void* ptr, std::size_t size);
    
            /* used for TESTS and DEBUG */
            
            std::vector<unsigned char>* memory() { return &_memory; }
    
            std::vector<OrderBlocks>* orderBlocksList() { return &_orderBlocksList; }
    
            std::size_t totalOrder() { return _totalOrder; }
            
            std::size_t totalMemory() { return _totalMemory; }
    };
}
#endif
