#include <cstddef>

#include "ThreadState.h"

namespace redhawk {
    namespace shm {

        class HeapPolicy
        {
        public:
            virtual int getHeapCount() = 0;

            virtual size_t getHeapAssignment(ThreadState* state) = 0;

            virtual void initThreadState(ThreadState*)
            {
                // No default behavior
            }
        };

        /**
         * CPU-based heap assignment policy.
         *
         * The total number of private heaps is a factor of the number of
         * CPUs, defaulting to a 1:1 mapping. This can be configured to alias
         * more CPUs to each heap to reduce overall memory footprint. On an
         * allocation, the current CPU is used to determine which private heap
         * will be used.
         */
        class CPUHeapPolicy : public HeapPolicy
        {
        public:
            CPUHeapPolicy();

            virtual int getHeapCount();

            virtual size_t getHeapAssignment(ThreadState*);

        private:
            size_t _cpusPerHeap;

            static size_t _getCpusPerHeap();
        };

        /**
         * Thread-based heap assignment policy.
         *
         * The total number of private heaps is configurable, defaulting to
         * the number of CPUs. Threads are assigned to one of the private
         * heaps based on their thread ID.
         */
        class ThreadHeapPolicy : public HeapPolicy
        {
        public:
            ThreadHeapPolicy();

            virtual int getHeapCount();

            virtual size_t getHeapAssignment(ThreadState* state);

            virtual void initThreadState(ThreadState* state);

        private:
            size_t _numHeaps;

            static size_t _getNumHeaps();
        };
    }
}
