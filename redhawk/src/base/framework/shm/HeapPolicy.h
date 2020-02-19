#include <cstddef>

#include "ThreadState.h"

namespace redhawk {
    namespace shm {

        class HeapPolicy
        {
        public:
            virtual ~HeapPolicy() { }

            virtual int getPoolCount() = 0;

            virtual size_t getPoolAssignment(ThreadState* state) = 0;

            virtual void initThreadState(ThreadState*)
            {
                // No default behavior
            }
        };

        /**
         * CPU-based heap policy.
         *
         * The total number of pools is a factor of the number of CPUs,
         * defaulting to a 1:1 mapping. This can be configured to alias more
         * CPUs to each pool to reduce overall memory footprint. On an
         * allocation, the current CPU is used to determine which pool will be
         * used.
         */
        class CPUHeapPolicy : public HeapPolicy
        {
        public:
            CPUHeapPolicy();

            virtual int getPoolCount();

            virtual size_t getPoolAssignment(ThreadState*);

        private:
            size_t _cpusPerPool;

            static size_t _getCpusPerPool();
        };

        /**
         * Thread-based heap policy.
         *
         * The total number of pools is configurable, defaulting to the number
         * of CPUs. Threads are assigned to one of the pools based on their
         * thread ID.
         */
        class ThreadHeapPolicy : public HeapPolicy
        {
        public:
            ThreadHeapPolicy();

            virtual int getPoolCount();

            virtual size_t getPoolAssignment(ThreadState* state);

            virtual void initThreadState(ThreadState* state);

        private:
            size_t _numPools;

            static size_t _getNumPools();
        };
    }
}
