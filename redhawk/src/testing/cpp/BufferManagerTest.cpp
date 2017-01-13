/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include "BufferManagerTest.h"
#include <ossie/ExecutorService.h>

#include <boost/thread.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION(BufferManagerTest);

void BufferManagerTest::setUp()
{
    _manager = &redhawk::BufferManager::Instance();

    // (Re-)enable the buffer manager
    _manager->enable(true);

    // Reset cache policies
    _manager->setMaxThreadBytes(-1);
    _manager->setMaxThreadBlocks(-1);
    _manager->setMaxThreadAge(-1);
}

void BufferManagerTest::tearDown()
{
    // Clean up all allocations from the test
    std::for_each(_allocations.begin(), _allocations.end(), &redhawk::BufferManager::Deallocate);
    _allocations.clear();

    // Disable the buffer manager to return all memory to the operating system
    _manager->enable(false);
}

void BufferManagerTest::testBasicAllocate()
{
    // Make sure a simple allocation succeeds, then return it to the cache
    const size_t SMALL_BYTES = 1000;
    void* buffer = _allocate(SMALL_BYTES);
    CPPUNIT_ASSERT(buffer != 0);
    _deallocate(buffer);

    // Allocate a different-sized buffer that cannot re-use the prior buffer
    const size_t LARGE_BYTES = 128*1024;
    void* large_buffer = _allocate(LARGE_BYTES);
    CPPUNIT_ASSERT(large_buffer != 0);
    CPPUNIT_ASSERT(buffer != large_buffer);

    // Allocate the original size and check that it returned the same buffer
    void* buffer2 = _allocate(SMALL_BYTES);
    CPPUNIT_ASSERT_EQUAL(buffer, buffer2);

    // Release the large buffer, and allocate another block of the smaller size
    _deallocate(large_buffer);
    buffer2 = _allocate(SMALL_BYTES);
    CPPUNIT_ASSERT(buffer2 != 0);
    CPPUNIT_ASSERT(buffer2 != buffer);
    CPPUNIT_ASSERT(buffer2 != large_buffer);
}

void BufferManagerTest::testAllocator()
{
    typedef std::vector<float,redhawk::BufferManager::Allocator<float> > FloatVec;

    // Create a 1K-element vector; the allocation had better succeed
    FloatVec vec;
    vec.resize(1024);
    float* buffer = vec.data();
    CPPUNIT_ASSERT(buffer != 0);

    // Resize vector up enough that it gets a different allocation
    const size_t ELEMENT_COUNT = 65536;
    vec.resize(ELEMENT_COUNT);
    float* large_buffer = vec.data();
    CPPUNIT_ASSERT(large_buffer != buffer);

    // Clear the vector's buffer
    size_t blocks_pre = _manager->getStatistics().blocks;
    {
        // Swapping with a new, empty vector is the most reliable way to reset
        // the vector's internal buffer; when the temporary gets destroyed at
        // the end of the scope, it should deallocate the buffer
        FloatVec tmp;
        vec.swap(tmp);
    }
    CPPUNIT_ASSERT(vec.data() != large_buffer);
    size_t blocks_post = _manager->getStatistics().blocks;
    CPPUNIT_ASSERT_EQUAL(blocks_pre + 1, blocks_post);

    // Resize back up to the last used size and check that we got the same
    // buffer back
    vec.resize(ELEMENT_COUNT);
    CPPUNIT_ASSERT_EQUAL(large_buffer, vec.data());
}

void BufferManagerTest::testEnable()
{
    // Start enabled and check that it reports true
    _manager->enable(true);
    CPPUNIT_ASSERT(_manager->isEnabled());

    // Disable the buffer manager and check that it reports false
    _manager->enable(false);
    CPPUNIT_ASSERT_EQUAL(false, _manager->isEnabled());

    // The cache(s) should be empty
    redhawk::BufferManager::Statistics stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL((size_t) 0, stats.bytes);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, stats.blocks);

    // Allocate and deallocate some buffers; the cache(s) should still be empty
    _fillCache(16, 8192);
    stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL((size_t) 0, stats.bytes);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, stats.blocks);

    // Re-enable and allocate more buffers
    _manager->enable(true);
    _fillCache(16, 8192);
    stats = _manager->getStatistics();
    CPPUNIT_ASSERT(stats.bytes >= (16*8192));
    CPPUNIT_ASSERT_EQUAL((size_t) 16, stats.blocks);

    // Disable the buffer manager again; it should purge the caches
    _manager->enable(false);
    stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL((size_t) 0, stats.bytes);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, stats.blocks);
}

void BufferManagerTest::testStatistics()
{
    // The cache should be clear (tearDown is supposed to clear it for us)
    redhawk::BufferManager::Statistics pre_stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL((size_t) 0, pre_stats.blocks);
    CPPUNIT_ASSERT_EQUAL((size_t) 0, pre_stats.bytes);

    // Allocate in increasing sizes; each one should be a cache miss
    size_t total_blocks = 8;
    size_t total_bytes = 0;
    size_t current_size = 1024;
    for (size_t ii = 0; ii < total_blocks; ++ii) {
        _deallocate(_allocate(current_size));
        total_bytes += current_size;
        current_size <<= 1;
    }
    redhawk::BufferManager::Statistics post_stats = _manager->getStatistics();
    size_t hits = post_stats.hits - pre_stats.hits;
    size_t misses = post_stats.misses - pre_stats.misses;
    CPPUNIT_ASSERT_EQUAL((size_t) 0, hits);
    CPPUNIT_ASSERT_EQUAL((size_t) total_blocks, misses);
    CPPUNIT_ASSERT(post_stats.blocks >= total_blocks);
    CPPUNIT_ASSERT(post_stats.bytes >= total_bytes);

    // Allocate a few blocks of sizes that should be in the cache; they should
    // all be hits
    current_size = 2048;
    for (size_t ii = 0; ii < 4; ++ii) {
        pre_stats = post_stats;
        _deallocate(_allocate(current_size));
        current_size <<= 2;
        post_stats = _manager->getStatistics();
        hits = post_stats.hits - pre_stats.hits;
        misses = post_stats.misses - pre_stats.misses;
        CPPUNIT_ASSERT_EQUAL((size_t) 1, hits);
        CPPUNIT_ASSERT_EQUAL((size_t) 0, misses);
    }

    // Exceed the high water mark
    pre_stats = post_stats;
    size_t gap = std::min(pre_stats.highBytes - pre_stats.bytes, (size_t) 1024);
    size_t count = (gap / 1024) + 2;
    _fillCache(count, 1024);
    post_stats = _manager->getStatistics();
    CPPUNIT_ASSERT(post_stats.highBytes > pre_stats.highBytes);
}

void BufferManagerTest::testThreading()
{
    // Pre-cache a buffer on the current thread
    _fillCache(1, 1024);
    redhawk::BufferManager::Statistics pre_stats = _manager->getStatistics();

    // Allocate a buffer on the executor service's thread
    redhawk::ExecutorService service;
    service.start();
    boost::packaged_task<void*> task(boost::bind(&BufferManagerTest::_allocate, this, 1024));
    boost::unique_future<void*> future = task.get_future();
    service.execute(boost::ref(task));
    void* buffer = future.get();

    // It should be a new buffer, not one from the cache; also, a new cache
    // should have been created
    redhawk::BufferManager::Statistics post_stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL(pre_stats.caches + 1, post_stats.caches);
    CPPUNIT_ASSERT_EQUAL(pre_stats.blocks, post_stats.blocks);
    CPPUNIT_ASSERT_EQUAL(pre_stats.bytes, post_stats.bytes);

    // Free the buffers allocated on the other thread; it should go back into
    // the executor thread's cache
    _deallocate(buffer);
    post_stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL(pre_stats.blocks + 1, post_stats.blocks);
    size_t delta = post_stats.bytes - pre_stats.bytes;
    CPPUNIT_ASSERT(delta >= 1024);

    // Allocate two buffers on the current thread; only one of them should come
    // from the cache--the other buffer is in the executor thread's cache
    CPPUNIT_ASSERT_EQUAL((size_t) 2, post_stats.blocks);
    pre_stats = post_stats;
    void* buffer1 = _allocate(1024);
    void* buffer2 = _allocate(1024);
    post_stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL(pre_stats.blocks - 1, post_stats.blocks);

    // Deallocate both buffers on the executor thread
    pre_stats = post_stats;
    boost::packaged_task<void> deallocate1(boost::bind(&BufferManagerTest::_deallocate, this, buffer1));
    boost::packaged_task<void> deallocate2(boost::bind(&BufferManagerTest::_deallocate, this, buffer2));
    service.execute(boost::ref(deallocate1));
    service.execute(boost::ref(deallocate2));
    deallocate1.get_future().wait();
    deallocate2.get_future().wait();

    // Make sure that the buffers returned to the cache
    post_stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL(pre_stats.blocks + 2, post_stats.blocks);
    CPPUNIT_ASSERT(post_stats.bytes > pre_stats.bytes);

    // End the executable service's thread, which should free that cache; only
    // the buffer that was *allocated* on the thread should be freed
    pre_stats = post_stats;
    service.stop();
    post_stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL(pre_stats.caches - 1, post_stats.caches);
    CPPUNIT_ASSERT_EQUAL(pre_stats.blocks - 1, post_stats.blocks);
    CPPUNIT_ASSERT(pre_stats.bytes > post_stats.bytes);
}

void BufferManagerTest::testPolicyBytes()
{
    // Fill the cache with more than 64K worth of buffers
    const size_t SMALL_BYTES = 8192;
    const size_t SMALL_LIMIT = 65536;
    _fillCache((SMALL_LIMIT / SMALL_BYTES) + 1, SMALL_BYTES);
    CPPUNIT_ASSERT(_manager->getStatistics().bytes > SMALL_LIMIT);

    // Limit the cache to 64K; this should free some buffers
    _manager->setMaxThreadBytes(65536);
    CPPUNIT_ASSERT_EQUAL((size_t) 65536, _manager->getMaxThreadBytes());
    CPPUNIT_ASSERT(_manager->getStatistics().bytes <= _manager->getMaxThreadBytes());

    // Allocate a couple of buffers and make sure they came from the cache
    size_t blocks = _manager->getStatistics().blocks;
    void* buffer1 = _allocate(SMALL_BYTES);
    void* buffer2 = _allocate(SMALL_BYTES);
    CPPUNIT_ASSERT(_manager->getStatistics().blocks < blocks);

    // Bring the limit down a little more; this shouldn't change the cache
    // size
    const size_t TEST_LIMIT = SMALL_LIMIT - (SMALL_BYTES/2);
    CPPUNIT_ASSERT(_manager->getStatistics().bytes <= TEST_LIMIT);
    size_t pre_bytes = _manager->getStatistics().bytes;
    _manager->setMaxThreadBytes(TEST_LIMIT);
    size_t post_bytes = _manager->getStatistics().bytes;
    CPPUNIT_ASSERT_EQUAL(pre_bytes, post_bytes);

    // Return one buffer to the cache; this should succeed and remain below the
    // limit
    pre_bytes = _manager->getStatistics().bytes;
    _deallocate(buffer1);
    post_bytes = _manager->getStatistics().bytes;
    CPPUNIT_ASSERT(post_bytes > pre_bytes);
    CPPUNIT_ASSERT(post_bytes <= _manager->getMaxThreadBytes());

    // Deallocate the second; this would exceed the limit, so the cache should
    // free it
    pre_bytes = post_bytes;
    _deallocate(buffer2);
    post_bytes = _manager->getStatistics().bytes;
    CPPUNIT_ASSERT_EQUAL(pre_bytes, post_bytes);

    // Increase the max bytes and try to fill past the limit; the cached bytes
    // should not exceed the limit, but the should be within a buffer size
    // (otherwise it would have accepted the buffer)
    const size_t LARGE_LIMIT = 1024 * 1024;
    _manager->setMaxThreadBytes(LARGE_LIMIT);
    const size_t LARGE_BYTES = 128*1024;
    _fillCache(10, LARGE_BYTES);
    redhawk::BufferManager::Statistics stats = _manager->getStatistics();
    CPPUNIT_ASSERT(stats.bytes <= LARGE_LIMIT);
    CPPUNIT_ASSERT((LARGE_LIMIT - stats.bytes) < LARGE_BYTES);

    // Turn off the byte policy, retaining all buffers from now on
    _manager->setMaxThreadBytes(-1);
    size_t pre_blocks = stats.blocks;
    _fillCache(10, 65536);
    stats = _manager->getStatistics();
    CPPUNIT_ASSERT(stats.bytes > LARGE_LIMIT);
    CPPUNIT_ASSERT_EQUAL(pre_blocks + 10, stats.blocks);
}

void BufferManagerTest::testPolicyBlocks()
{
    // Fill the cache with a set number of buffers
    const size_t SMALL_BYTES = 8192;
    size_t block_count = 12;
    _fillCache(block_count, SMALL_BYTES);
    CPPUNIT_ASSERT(_manager->getStatistics().blocks >= block_count);

    // Limit the cache to 8 block; this should free some buffers
    block_count = 8;
    _manager->setMaxThreadBlocks(block_count);
    CPPUNIT_ASSERT_EQUAL(block_count, _manager->getMaxThreadBlocks());
    CPPUNIT_ASSERT_EQUAL(block_count, _manager->getStatistics().blocks);

    // Allocate a couple of buffers and make sure they came from the cache
    void* buffer1 = _allocate(SMALL_BYTES);
    void* buffer2 = _allocate(SMALL_BYTES);
    CPPUNIT_ASSERT(_manager->getStatistics().blocks < block_count);

    // Bring the limit down a little more; this shouldn't change the cache
    // size
    block_count = 7;
    size_t pre_blocks = _manager->getStatistics().blocks;
    _manager->setMaxThreadBlocks(block_count);
    CPPUNIT_ASSERT_EQUAL(pre_blocks, _manager->getStatistics().blocks);

    // Return one buffer to the cache; this should succeed and remain below the
    // limit
    _deallocate(buffer1);
    size_t post_blocks = _manager->getStatistics().blocks;
    CPPUNIT_ASSERT(post_blocks > pre_blocks);
    CPPUNIT_ASSERT(post_blocks <= _manager->getMaxThreadBlocks());

    // Deallocate the second; this would exceed the limit, so the cache should
    // free it
    pre_blocks = post_blocks;
    _deallocate(buffer2);
    CPPUNIT_ASSERT_EQUAL(pre_blocks, _manager->getStatistics().blocks);

    // Increase the block limit and allocate enough more buffers both hit the
    // limit and flush the old buffers
    block_count += 5;
    _manager->setMaxThreadBlocks(block_count);
    const size_t LARGE_BYTES = SMALL_BYTES * 4;
    _fillCache(block_count * 2, LARGE_BYTES);
    redhawk::BufferManager::Statistics stats = _manager->getStatistics();
    CPPUNIT_ASSERT_EQUAL(block_count, stats.blocks);
    CPPUNIT_ASSERT(stats.bytes >= (stats.blocks * LARGE_BYTES));

    // Turn off the block policy, retaining all buffers from now on
    _manager->setMaxThreadBlocks(-1);
    pre_blocks = stats.blocks;
    _fillCache(10, SMALL_BYTES);
    CPPUNIT_ASSERT_EQUAL(pre_blocks + 10, _manager->getStatistics().blocks);
}

void BufferManagerTest::testPolicyAge()
{
    const size_t SMALL_BYTES = 1024;  // 1K
    _fillCache(10, SMALL_BYTES);

    // Set a maximum thread age less than the current number of blocks and
    // check that some blocks are freed
    size_t pre_blocks = _manager->getStatistics().blocks;
    CPPUNIT_ASSERT_EQUAL((size_t) 10, pre_blocks);
    _manager->setMaxThreadAge(8);
    CPPUNIT_ASSERT_EQUAL((size_t) 8, _manager->getMaxThreadAge());
    CPPUNIT_ASSERT(_manager->getStatistics().blocks < pre_blocks);

    // Allocate a bunch of different-sized buffers; this should age off all of
    // the smaller buffers
    const size_t MEDIUM_BYTES = 65536; // 64K
    for (int ii = 0; ii < 2; ++ii) {
        _fillCache(5, MEDIUM_BYTES);
    }
    redhawk::BufferManager::Statistics stats = _manager->getStatistics();
    size_t post_blocks = stats.blocks;
    CPPUNIT_ASSERT_EQUAL((size_t) 5, post_blocks);
    CPPUNIT_ASSERT(stats.bytes >= (post_blocks * MEDIUM_BYTES));

    // Cycle another larger buffer to the front just enough times that the
    // oldest medium buffer is still in the cache
    pre_blocks = post_blocks;
    const size_t LARGE_BYTES = 1024*1024; // 1M
    for (int ii = 0; ii < 4; ++ii) {
        _deallocate(_allocate(LARGE_BYTES));
    }
    post_blocks = _manager->getStatistics().blocks;
    CPPUNIT_ASSERT(post_blocks > pre_blocks);

    // The next allocate/deallocate cycle should age off an old buffer
    pre_blocks = _manager->getStatistics().blocks;
    _deallocate(_allocate(LARGE_BYTES));
    post_blocks =  _manager->getStatistics().blocks;
    CPPUNIT_ASSERT(post_blocks < pre_blocks);

    // Turn off the age policy, retaining all buffers from now on
    pre_blocks = post_blocks;
    _manager->setMaxThreadAge(-1);
    for (int ii = 0; ii < 1000; ++ii) {
        _deallocate(_allocate(LARGE_BYTES));
    }
    CPPUNIT_ASSERT_EQUAL(pre_blocks, _manager->getStatistics().blocks);
}

void* BufferManagerTest::_allocate(size_t bytes)
{
    void* ptr = redhawk::BufferManager::Allocate(bytes);
    _allocations.insert(ptr);
    return ptr;
}

void BufferManagerTest::_deallocate(void* ptr)
{
    CPPUNIT_ASSERT_MESSAGE("Deallocating unknown allocation", _allocations.erase(ptr));
    redhawk::BufferManager::Deallocate(ptr);
}

void BufferManagerTest::_fillCache(size_t count, size_t bufferSize)
{
    BufferList buffers;
    for (size_t blocks = 0; blocks < count; ++blocks) {
        buffers.insert(_manager->allocate(bufferSize));
    }
    std::for_each(buffers.begin(), buffers.end(), &redhawk::BufferManager::Deallocate);
}
