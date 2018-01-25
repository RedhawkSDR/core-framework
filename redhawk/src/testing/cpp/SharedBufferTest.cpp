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

#include "SharedBufferTest.h"

#include <complex>

#define _RH_SHARED_BUFFER_USE_STD_ALLOC
#include <ossie/shared_buffer.h>

CPPUNIT_TEST_SUITE_REGISTRATION(SharedBufferTest);

namespace {
    // Special allocator that wraps an existing char buffer, marking allocated
    // data with ones and deallocated data with zeros.
    template <class T>
    struct CustomAllocator : public std::allocator<T>
    {
        CustomAllocator(char* buffer) :
            _buffer(buffer)
        {
        }

        T* allocate(size_t count, void* ptr=0)
        {
            size_t bytes = count * sizeof(T);
            std::fill(_buffer, _buffer + bytes, 1);
            return reinterpret_cast<T*>(_buffer);
        }

        void deallocate(void* ptr, size_t count)
        {
            size_t bytes = count * sizeof(T);
            std::fill(_buffer, _buffer + bytes, 0);
        }

    private:
        char* _buffer;
    };

    // Special deleter that writes to a boolean flag pointer to notify when it
    // has been called. Using a pointer-to-value instead of its own local flag
    // allows it to be passed by value, which is the preferred way of passing a
    // deleter to boost::shared_array (and by extension redhawk::shared_buffer)
    struct CustomDeleter {
        CustomDeleter(bool* deleted) :
            _deleted(deleted)
        {
        }

        void operator() (void* ptr)
        {
            std::free(ptr);
            *_deleted = true;
        }

        bool* _deleted;
    };

    struct NullDeleter {
        void operator() (void*)
        {
        }
    };
}

void SharedBufferTest::setUp()
{
}

void SharedBufferTest::tearDown()
{
}

void SharedBufferTest::testDefaultConstructor()
{
    // Empty const shared buffer
    const redhawk::shared_buffer<short> shared;
    CPPUNIT_ASSERT(shared.size() == 0);
    CPPUNIT_ASSERT(shared.empty());
    CPPUNIT_ASSERT_EQUAL(shared.begin(), shared.end());

    // Empty regular buffer
    redhawk::buffer<short> buffer;
    CPPUNIT_ASSERT(buffer.size() == 0);
    CPPUNIT_ASSERT(buffer.empty());
    CPPUNIT_ASSERT_EQUAL(buffer.begin(), buffer.end());
}

void SharedBufferTest::testConstructor()
{
    // Test allocating constructor
    const size_t BUFFER_SIZE = 16;
    redhawk::buffer<float> buffer(BUFFER_SIZE);
    CPPUNIT_ASSERT(!buffer.empty());
    CPPUNIT_ASSERT_EQUAL(buffer.size(), BUFFER_SIZE);
    CPPUNIT_ASSERT(buffer.begin() != buffer.end());

    // Test construction of shared buffer from mutable buffer
    redhawk::shared_buffer<float> shared(buffer);
    CPPUNIT_ASSERT(!shared.empty());
    CPPUNIT_ASSERT_EQUAL(shared.size(), buffer.size());
    CPPUNIT_ASSERT(shared.data() == buffer.data());
}

void SharedBufferTest::testMakeBuffer()
{
    // Simple wrapper for externally-allocated array
    int* array = new int[16];
    redhawk::buffer<int> buffer = redhawk::make_buffer(array, 16);
    CPPUNIT_ASSERT_EQUAL(array, buffer.data());

    // Use a custom deleter that is a no-op, with a stack-based array
    double darray[4] = { 1.0, 2.0, 3.0, 4.0 };
    redhawk::buffer<double> dbuffer = redhawk::make_buffer(darray, 4, NullDeleter());
    CPPUNIT_ASSERT_EQUAL(&darray[0], dbuffer.data());
}

void SharedBufferTest::testEquals()
{
    // Create a buffer with known data
    redhawk::buffer<long> first(6);
    std::fill(first.begin(), first.end(), 8);

    // Create a second, identical buffer and check that it is equal
    redhawk::buffer<long> second(first.size());
    std::copy(first.begin(), first.end(), second.begin());
    CPPUNIT_ASSERT(first.data() != second.data());
    CPPUNIT_ASSERT(first == second);

    // Modify an element, breaking equality
    second[3] = -25;
    CPPUNIT_ASSERT(first != second);

    // Re-allocate the second buffer with one extra element
    second = redhawk::buffer<long>(first.size() + 1);
    std::copy(first.begin(), first.end(), second.begin());
    second[second.size() - 1] = second[0];
    CPPUNIT_ASSERT(first != second);
}

void SharedBufferTest::testIteration()
{
    // Create a buffer with known data
    redhawk::buffer<unsigned long> buffer(20);
    for (unsigned long index = 0; index < buffer.size(); ++index) {
        buffer[index] = index;
    }

    // The distance between the begin and end iterators must be the same as the
    // size, and iteration should yield the same result as sequential indexing
    size_t offset = 0;
    for (redhawk::buffer<unsigned long>::iterator iter = buffer.begin(); iter != buffer.end(); ++iter, ++offset) {
        CPPUNIT_ASSERT_EQUAL(*iter, buffer[offset]);
    }
    CPPUNIT_ASSERT_EQUAL(buffer.size(), offset);

    // Repeat, via a const shared buffer alias
    const redhawk::shared_buffer<unsigned long> shared = buffer;
    CPPUNIT_ASSERT_EQUAL((ptrdiff_t) shared.size(), std::distance(shared.begin(), shared.end()));
    CPPUNIT_ASSERT(std::equal(shared.begin(), shared.end(), buffer.begin()));
}

void SharedBufferTest::testCopy()
{
    // Create a buffer with known data
    redhawk::buffer<bool> original(9);
    for (size_t index = 0; index < original.size(); ++index) {
        // Value is true if index is odd
        original[index] = index & ~1;
    }

    // Create a const shared buffer alias
    const redhawk::shared_buffer<bool> buffer = original;
    CPPUNIT_ASSERT(buffer.data() == original.data());

    // Make a copy, and verify that it's a new underlying buffer
    redhawk::buffer<bool> copy = buffer.copy();
    CPPUNIT_ASSERT(copy == buffer);
    CPPUNIT_ASSERT(copy.data() != buffer.data());

    // Make a second copy, exercising copy() from buffer (instead of
    // shared_buffer)
    redhawk::buffer<bool> second = copy.copy();
    CPPUNIT_ASSERT(second == buffer);
    CPPUNIT_ASSERT(second.data() != copy.data());
}

void SharedBufferTest::testSwap()
{
    // Create two mutable buffers with different contents
    redhawk::buffer<int> first(3);
    std::fill(first.begin(), first.end(), 7);
    CPPUNIT_ASSERT_EQUAL(first.size(), (size_t) std::count(first.begin(), first.end(), 7));
    redhawk::buffer<int> second(5);
    std::fill(second.begin(), second.end(), -2);
    CPPUNIT_ASSERT_EQUAL(second.size(), (size_t) std::count(second.begin(), second.end(), -2));

    // Swap them and check that the swap worked as expected
    first.swap(second);
    CPPUNIT_ASSERT(first.size() == 5);
    CPPUNIT_ASSERT(first[0] == -2);
    CPPUNIT_ASSERT(second.size() == 3);
    CPPUNIT_ASSERT(second[0] == 7);

    // Create one shared buffer alias for each buffer
    redhawk::shared_buffer<int> shared_first = first;
    CPPUNIT_ASSERT(shared_first.data() == first.data());
    redhawk::shared_buffer<int> shared_second = second;
    CPPUNIT_ASSERT(shared_second.data() == second.data());

    // Swap the shared buffers and make sure that the underlying data pointers
    // are correct
    shared_first.swap(shared_second);
    CPPUNIT_ASSERT(shared_first.data() == second.data());
    CPPUNIT_ASSERT(shared_second.data() == first.data());
}

void SharedBufferTest::testResize()
{
    // Fill a new buffer with a ramp (offset by 1) for easy comparison
    redhawk::buffer<unsigned char> buffer(8);
    for (size_t index = 0; index < buffer.size(); ++index) {
        buffer[index] = index + 1;
    }

    // Resize the buffer and check that the values are preserved
    buffer.resize(16);
    CPPUNIT_ASSERT_EQUAL((size_t) 16, buffer.size());
    for (size_t index = 0; index < 8; ++index) {
        unsigned char expected = index + 1;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Resize did not copy data", expected, buffer[index]);
    }

    // Resize down (which can be done better with trim, but is still legal) and
    // check values
    buffer.resize(4);
    CPPUNIT_ASSERT_EQUAL((size_t) 4, buffer.size());
    for (size_t index = 0; index < buffer.size(); ++index) {
        unsigned char expected = index + 1;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Resize did not copy data", expected, buffer[index]);
    }
}

void SharedBufferTest::testSharing()
{
    // Fill a new buffer
    redhawk::buffer<std::complex<double> > buffer(8);
    for (size_t index = 0; index < buffer.size(); ++index) {
        buffer[index] = std::complex<double>(0.5, 0.5) * (double) index;
    }

    // Create a const shared buffer aliasing the original
    const redhawk::shared_buffer<std::complex<double> > shared = buffer;
    CPPUNIT_ASSERT(shared == buffer);

    // Conjugate values and ensure that the buffers are still equal
    std::transform(buffer.begin(), buffer.end(), buffer.begin(), std::conj<double>);
    CPPUNIT_ASSERT(shared == buffer);
}

void SharedBufferTest::testSlicing()
{
    // Fill a new buffer
    redhawk::buffer<short> buffer(12);
    for (size_t index = 0; index < buffer.size(); ++index) {
        buffer[index] = index;
    }

    // Take a 4-element slice from the middle and check that it points to the
    // same data (offset by the start index)
    const redhawk::shared_buffer<short> middle = buffer.slice(4, 8);
    CPPUNIT_ASSERT_EQUAL(middle.size(), (size_t) 4);
    CPPUNIT_ASSERT(middle.data() == buffer.data() + 4);

    // Take a slice from the midpoint to the end, and check that the elements
    // match
    const redhawk::shared_buffer<short> end = buffer.slice(6);
    CPPUNIT_ASSERT_EQUAL(end.size(), buffer.size() - 6);
    for (size_t index = 0; index < end.size(); ++index) {
        CPPUNIT_ASSERT_EQUAL(end[index], buffer[index + 6]);
    }

    // Compare the overlap between the two slices by taking sub-slices
    CPPUNIT_ASSERT(middle.slice(2) == end.slice(0, 2));
}

template <class Buffer>
void SharedBufferTest::testTrimImpl()
{
    redhawk::buffer<typename Buffer::value_type> original(10);
    for (size_t index = 0; index < original.size(); ++index) {
        original[index] = index;
    }

    // Create an alias of the template type, then trim one element off each end
    Buffer buffer = original;
    buffer.trim(1, original.size() - 1);
    CPPUNIT_ASSERT_EQUAL(buffer.size(), original.size() - 2);
    CPPUNIT_ASSERT(std::equal(buffer.begin(), buffer.end(), original.begin() + 1));

    // Trim another element off the beginning
    buffer.trim(1);
    CPPUNIT_ASSERT_EQUAL(buffer.size(), original.size() - 3);
    CPPUNIT_ASSERT(std::equal(buffer.begin(), buffer.end(), original.begin() + 2));

    // Iterator-based trim: find specific values and trim to [first, last)
    typename Buffer::iterator first = std::find(buffer.begin(), buffer.end(), 4);
    typename Buffer::iterator last = std::find(buffer.begin(), buffer.end(), 7);
    CPPUNIT_ASSERT(first != buffer.end());
    CPPUNIT_ASSERT(last != buffer.end());
    buffer.trim(first, last);
    CPPUNIT_ASSERT(buffer.size() == 3);
    CPPUNIT_ASSERT_EQUAL(buffer[0], (unsigned short) 4);

    // Use iterator-based trim to take another element off the beginning
    buffer.trim(buffer.begin() + 1);
    CPPUNIT_ASSERT(buffer.size() == 2);
    CPPUNIT_ASSERT_EQUAL(buffer[0], (unsigned short) 5);
}

void SharedBufferTest::testTrim()
{
    testTrimImpl<redhawk::buffer<unsigned short> >();
}

void SharedBufferTest::testTrimShared()
{
    testTrimImpl<redhawk::shared_buffer<unsigned short> >();
}

void SharedBufferTest::testReplace()
{
    // Fill destination with a constant value
    redhawk::buffer<int> dest(24);
    std::fill(dest.begin(), dest.end(), 6789);

    // Create a different pattern for the replacement data
    redhawk::buffer<int> pattern(4);
    for (size_t ii = 0; ii < pattern.size(); ++ii) {
        pattern[ii] = (ii + 1) * 1000;
    }
    const redhawk::shared_buffer<int> src = pattern;

    // 3-argument version: replace 3 elements at offset 1
    dest.replace(1, 3, src);
    CPPUNIT_ASSERT_EQUAL(6789, dest[0]);
    CPPUNIT_ASSERT_EQUAL(1000, dest[1]);
    CPPUNIT_ASSERT_EQUAL(2000, dest[2]);
    CPPUNIT_ASSERT_EQUAL(3000, dest[3]);
    CPPUNIT_ASSERT_EQUAL(6789, dest[4]);

    // 4-argument version: replace 2 elements at offset 16, starting at offset
    // 2 in the source
    dest.replace(16, 2, src, 2);
    CPPUNIT_ASSERT_EQUAL(6789, dest[15]);
    CPPUNIT_ASSERT_EQUAL(3000, dest[16]);
    CPPUNIT_ASSERT_EQUAL(4000, dest[17]);
    CPPUNIT_ASSERT_EQUAL(6789, dest[18]);
}

void SharedBufferTest::testRecast()
{
    // Fill a new buffer with an odd number of elements
    redhawk::buffer<float> buffer(13);
    for (size_t index = 0; index < buffer.size(); ++index) {
        buffer[index] = index * 2.0 * M_PI;
    }

    // Recast to the complex equivalent, and check that its size is determined
    // using floor division (i.e., the extra real value is excluded)
    redhawk::shared_buffer<std::complex<float> > cxbuffer;
    cxbuffer = redhawk::shared_buffer<std::complex<float> >::recast(buffer);
    CPPUNIT_ASSERT_EQUAL(cxbuffer.size(), buffer.size() / 2);
    CPPUNIT_ASSERT_EQUAL(cxbuffer.size() * 2, buffer.size() - 1);

    // Check that the complex buffer has the original values interleaved as
    // real/imaginary pairs
    for (size_t index = 0; index < (cxbuffer.size() * 2); index += 2) {
        CPPUNIT_ASSERT_EQUAL(cxbuffer[index/2].real(), buffer[index]);
        CPPUNIT_ASSERT_EQUAL(cxbuffer[index/2].imag(), buffer[index+1]);
    }

    // Recast into short, this time using buffer::recast (such that the result
    // is mutable); this is basically nonsensical, but technically valid, so
    // only check that the data pointer and size are correct
    redhawk::buffer<short> short_buffer;
    short_buffer = redhawk::buffer<short>::recast(buffer);
    CPPUNIT_ASSERT_EQUAL((void*) short_buffer.data(), (void*) buffer.data()); 
    CPPUNIT_ASSERT_EQUAL(short_buffer.size(), buffer.size() * 2); 
}

void SharedBufferTest::testAllocator()
{
    typedef std::complex<short> value_type;
    typedef CustomAllocator<value_type> allocator_type;

    // Start with a zero-filled buffer of "raw" memory
    std::vector<char> arena;
    arena.resize(32);

    // The initial condition of the arena should be all zeros
    std::fill(arena.begin(), arena.end(), 0);
    CPPUNIT_ASSERT_EQUAL(arena.size(), (size_t) std::count(arena.begin(), arena.end(), 0));

    // Create a new buffer using a custom allocator; we should see that it's
    // marked the allocated space with ones
    redhawk::buffer<value_type> buffer(4, allocator_type(&arena[0]));
    const size_t MIN_BYTES = sizeof(value_type) * buffer.size();
    size_t index = 0;
    while ((index < arena.size()) && (arena[index] == 1)) {
        ++index;
    }
    CPPUNIT_ASSERT(index >= MIN_BYTES);

    // Release the buffer, which should trigger a deallocation; we should see
    // that it's reset the allocated space to zeros
    buffer = redhawk::buffer<value_type>();
    CPPUNIT_ASSERT_EQUAL(arena.size(), (size_t) std::count(arena.begin(), arena.end(), 0));
}

void SharedBufferTest::testAllocatorCopy()
{
    typedef CustomAllocator<double> allocator_type;

    // Initialize source buffer
    redhawk::buffer<double> buffer(16);
    for (size_t index = 0; index < buffer.size(); ++index) {
        buffer[index] = index * 2.0 * M_PI / buffer.size();
    }

    // Over-allocate memory for the custom allocator, just to be safe
    std::vector<char> arena;
    arena.resize(buffer.size() * sizeof(double) * 2);

    // Create a copy using the custom allocator, then ensure that the copy was
    // allocated into our memory (and the copy is correct)
    redhawk::shared_buffer<double> copy = buffer.copy(allocator_type(&arena[0]));
    CPPUNIT_ASSERT(reinterpret_cast<const void*>(copy.data()) == &arena[0]);
    CPPUNIT_ASSERT(copy == buffer);
}

void SharedBufferTest::testCustomDeleter()
{
    // Local flag given to the custom deleter for checking when deletion has
    // occurred
    bool deleted = false;

    // Allocate a buffer through other means, then wrap it with a buffer using
    // a custom deleter
    char* data = static_cast<char*>(std::malloc(16));
    redhawk::shared_buffer<char> buffer(data, sizeof(data), CustomDeleter(&deleted));
    CPPUNIT_ASSERT(buffer.data() == data);

    // Make a new shared alias and replace the first buffer; there is still one
    // outstanding reference, so the deleter should not be called
    redhawk::shared_buffer<char> shared = buffer;
    buffer = redhawk::shared_buffer<char>();
    CPPUNIT_ASSERT(!deleted);

    // Replace the second buffer, which should be the last reference; the
    // deleter should be called
    shared = redhawk::shared_buffer<char>();
    CPPUNIT_ASSERT(deleted);
}

void SharedBufferTest::testTransient()
{
    // Wrap a transient shared buffer around a C-style array
    int data[] = { 8, 6, 7, 5, 3, 0, 9 };
    const size_t BUFFER_SIZE = sizeof(data) / sizeof(data[0]);
    redhawk::shared_buffer<int> buffer = redhawk::shared_buffer<int>::make_transient(data, BUFFER_SIZE);
    CPPUNIT_ASSERT(buffer.transient());
    CPPUNIT_ASSERT(buffer.data() == data);
    CPPUNIT_ASSERT(buffer.size() == BUFFER_SIZE);
    for (size_t index = 0; index < buffer.size(); ++index) {
        CPPUNIT_ASSERT_EQUAL(buffer[index], data[index]);
    }

    // Allocate a new buffer and assign it back to our original buffer, making
    // sure that it is no longer transient
    buffer = redhawk::buffer<int>(1);
    CPPUNIT_ASSERT(!buffer.transient());
}

void SharedBufferTest::testGetMemory()
{
    // Using externally-acquired memory, check that the shared buffer's data()
    // and base() values match the original value
    const size_t BUFFER_SIZE = 12;
    float* data = new float[BUFFER_SIZE];
    redhawk::shared_buffer<float> buffer(data, BUFFER_SIZE);
    CPPUNIT_ASSERT_EQUAL((const void*) data, buffer.get_memory().address());
    CPPUNIT_ASSERT_EQUAL((const void*) buffer.data(), buffer.get_memory().address());

    // Recasting shouldn't affect the base pointer
    redhawk::shared_buffer<int> alias = redhawk::shared_buffer<int>::recast(buffer);
    CPPUNIT_ASSERT_EQUAL((const void*) data, alias.get_memory().address());

    // Taking a slice of the original buffer should produce a different data
    // pointer but preserve the base pointer
    redhawk::shared_buffer<float> slice = buffer.slice(2, 7);
    const void* slice_base = slice.get_memory().address();
    CPPUNIT_ASSERT(slice.data() != slice_base);
    CPPUNIT_ASSERT_EQUAL((const void*) data, slice_base);

    // Recast the slice, which should still preserve the base pointer
    alias = redhawk::shared_buffer<int>::recast(slice);
    CPPUNIT_ASSERT_EQUAL((const void*) data, alias.get_memory().address());

    // Transient buffers should always return null
    redhawk::shared_buffer<float> transient = redhawk::shared_buffer<float>::make_transient(data, BUFFER_SIZE);
    CPPUNIT_ASSERT_EQUAL((const void*) 0, transient.get_memory().address());
}
