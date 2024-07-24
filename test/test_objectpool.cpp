#include <gtest/gtest.h>

#include <vector>
#include <iostream>

#include "memorypool.hpp"

class TestClass
{
    int a;
    char b;
    float c;

    char d[1300];
};

class ObjectPoolTest : public testing::Test
{
    protected:
        static constexpr std::size_t _size = 1000;
        Memory::ObjectPool<TestClass> _objectPool;

        ObjectPoolTest() : _objectPool{_size} {}
};

TEST_F(ObjectPoolTest, Constructor)
{
    auto memPtr = _objectPool.memory();

    ASSERT_EQ(memPtr->size(), _size);
}

TEST_F(ObjectPoolTest, normalAllocation)
{
    auto memPtr = _objectPool.memory();
    auto stackPtr = _objectPool.freeBlocks();
    std::size_t topBlock = stackPtr->top();

    TestClass* object = _objectPool.allocate();

    ASSERT_NE(topBlock, stackPtr->top());
    ASSERT_EQ(&((*memPtr)[topBlock]), object);
}

TEST_F(ObjectPoolTest, normalDeallocation)
{
    auto stackPtr = _objectPool.freeBlocks();
    std::size_t topBlock = stackPtr->top();

    TestClass* object = _objectPool.allocate();
    _objectPool.deallocate(object);
    
    ASSERT_EQ(topBlock, stackPtr->top());
}

TEST_F(ObjectPoolTest, overAllocation)
{
    std::vector<TestClass*> vec(_size);

    for (std::size_t i = 0; i < _size; ++i) {
        vec[i] = _objectPool.allocate();       
    }

    ASSERT_THROW({
        try {
         
            _objectPool.allocate();
        }
        catch( const std::runtime_error& err) {
        
            ASSERT_STREQ("memorypool.cpp: Memory::ObjectPool<T>::allocate(): there aren't any free blocks", err.what());
            throw;
        }
    }, std::runtime_error);


    for (std::size_t i = 0; i < _size; ++i) {
        _objectPool.deallocate(vec[i]);       
    }
}

TEST_F(ObjectPoolTest, nullDeallocation)
{
    TestClass* object = nullptr;
    ASSERT_THROW({
        try {
            _objectPool.deallocate(object);        
        }
        catch( const std::runtime_error& err) {
        
            ASSERT_STREQ("memorypool.cpp: Memory::ObjectPool<T>::deallocate(): block is nullptr", err.what());
            throw;
        }
    }, std::runtime_error);
}

#define PERFORMANCE_TESTS   1000

TEST_F(ObjectPoolTest, objectPoolPerformance)
{
    for (int i = 0; i < PERFORMANCE_TESTS; ++i) {
        TestClass* object = _objectPool.allocate();       
        _objectPool.deallocate(object);
    }
}

TEST_F(ObjectPoolTest, DefaultNewPerformance)
{
    for (int i = 0; i < PERFORMANCE_TESTS; ++i) {
        TestClass* object = new TestClass();       
        delete object;
    }
}
