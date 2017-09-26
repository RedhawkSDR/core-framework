#ifndef SHMARENA_H
#define SHMARENA_H

#include <ossie/shm/shared_mutex.h>

#include <ostream>
#include <inttypes.h>

namespace redhawk {

    class ShmBlock;
    class ThreadState;

    class ShmArena {
    public:
        ShmArena(size_t offset, size_t size);
        ~ShmArena();

        size_t offset() const;

        size_t size() const;

        void* allocate(ThreadState* thread, size_t bytes);

        void* attach(size_t offset);

        static void deallocate(void* ptr);

        void dump(std::ostream& stream) const;

    protected:
        struct FreeBlock;

        void _deallocate(ShmBlock* block);

        void _dump(std::ostream& stream) const;

        char* _data();
        const char* _data() const;
        const ShmBlock* _begin() const;
        const ShmBlock* _end() const;
        void _queueFree(ShmBlock* block);

        FreeBlock* _findAvailable(size_t bytes);
        void _removeFreeBlock(FreeBlock* block);

        void _coalesceNext(ShmBlock* block);
        ShmBlock* _coalescePrevious(ShmBlock* block);

        FreeBlock* _offsetToBlock(uint32_t offset);
        const FreeBlock* _offsetToBlock(uint32_t offset) const;

        const uint64_t _offset;
        const uint32_t _size;
        const uint32_t _dataStart;
        mutable redhawk::shared_mutex _lock;

        // Free list pointers
        uint32_t _first;
        uint32_t _last;
    };
}

#endif // SHMARENA_H
