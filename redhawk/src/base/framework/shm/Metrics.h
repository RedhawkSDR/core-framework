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

#ifndef REDHAWK_SHM_METRICS_H
#define REDHAWK_SHM_METRICS_H

#include <ostream>
#include <cstddef>

#include "atomic_counter.h"

namespace redhawk {
    namespace shm {
        struct Metrics {
            typedef atomic_counter<int> atomic_int;

            // SuperblockFile statistics
            /**
             * Number of superblock files created by this process.
             */
            atomic_int files_created;

            /**
             * Number of superblock files opened by this process.
             *
             * Indicates the number of other processes' shared memory heaps to
             * which this process has attached.
             */
            atomic_int files_opened;

            /**
             * Number of superblock files closed by this process.
             *
             * At process exit, this number should be equal to the number of
             * superblock files created plus opened.
             */
            atomic_int files_closed;

            /**
             * Total number of bytes allocated in superblock files created by
             * this process.
             */
            atomic_counter<size_t> files_bytes;

            // Heap statistics
            /**
             * Number of heaps created by this process.
             *
             * The number of heaps created should be equal to the number of
             * superblock files created.
             */
            atomic_int heaps_created;

            /**
             * Number of pools created in this process.
             *
             * Pool creation occurs at heap initialization time. No
             * shared memory is dedicated to a pool until it is used for
             * allocation.
             */
            atomic_int pools_created;

            /**
             * Number of pools in use by this process.
             *
             * Indicates the actual usage pattern of pools.
             */
            atomic_int pools_used;

            /**
             * Number of pool allocations that were satisfied by an
             * existing superblock.
             *
             * A high number of "hot" allocations suggests that memory is being
             * allocated efficiently. Accordingly, the virtual address space and
             * superblock file will tend to grow more slowly.
             */
            atomic_int pools_alloc_hot;

            /**
             * Number of pool allocations that required acquiring a new
             * superblock.
             *
             * A high number of "cold" allocations suggests either heavy memory
             * use or inefficient allocation of memory. The virtual address
             * space and superblock file will tend to grow more quickly.
             */
            atomic_int pools_alloc_cold;

            /**
             * Number of pool allocations that were not able to be
             * fulfilled.
             *
             * A failed allocation indicates that the shared memory filesystem
             * is full.
             */
            atomic_int pools_alloc_failed;

            // Superblock statistics
            /**
             * Number of superblocks created by this process.
             *
             * A superblock is created by a heap owned by this process. A higher
             * number of superblocks created indicates more shared memory usage.
             */
            atomic_int superblocks_created;

            /**
             * Number of superblocks mapped into this process' memory space.
             *
             * A mapped superblocks references another process' heap. A higher
             * number of superblocks mapped indicates more virtual address space
             * usage.
             */
            atomic_int superblocks_mapped;

            /**
             * Number of superblock mappings that were reused.
             *
             * A superblock is reused whenever a mapping is requested that has
             * already been fulfilled. A high ratio of reused to mapped suggests
             * efficient allocation.
             */
            atomic_int superblocks_reused;

            /**
             * Number of superblocks unmapped from this process' memory space.
             *
             * Currently, superblocks are never unmapped. This number will
             * always be zero.
             */
            atomic_int superblocks_unmapped;

            // HeapClient statistics
            /**
             * Number of heap clients created.
             *
             * Each heap client maintains its own mapping of heaps from other
             * processes. No state is shared between heap clients, meaning that
             * the same superblock may be mapped multiple times in a single
             * process with more than one heap client.
             */
            atomic_int clients_created;

            /**
             * Number of heap clients destroyed.
             *
             * In practice, at process exit this should be equal to the number
             * of heap clients created.
             */
            atomic_int clients_destroyed;

            // Block statistics
            /**
             * Number of shared memory blocks allocated within this process.
             */
            atomic_int blocks_created;

            /**
             * Number of shared memory blocks to which this process has
             * attached.
             *
             * Each attach call increments the block's reference count. Across
             * the system, the same block may have several clients attach to it.
             */
            atomic_int blocks_attached;

            /**
             * Number of shared memory blocks from which this process has
             * detached.
             *
             * Each detach call decrements the block's reference count. If the
             * reference count dropped to zero when this process detached, it
             * was also destroyed by this process.
             *
             * For the entire system, the number of blocks released should equal
             * the total of blocks created and attached.
             */
            atomic_int blocks_released;

            /**
             * Number of shared memory blocks returned to their superblock's
             * free list within this process.
             *
             * Each block is created and destroyed exactly once across the
             * system. The process that destroys a block is not necessarily the
             * same process that created it.
             *
             * For the entire system, the number of blocks destroyed should
             * equal the number of blocks created.
             */
            atomic_int blocks_destroyed;

            /**
             * Returns the singleton instance.
             */
            static Metrics& Instance();

            ~Metrics();

            /**
             * Flag indicating whether metrics are being collected.
             *
             * Initialized at process startup.
             */
            static const bool enabled;

            void dump(std::ostream& oss);

        private:
            Metrics();

            static bool _initializeMetrics();

            static Metrics& _instance();
        };
    }
}

#define RECORD_SHM_METRIC_ADD_IF(COND,X,COUNT)                    \
    if (redhawk::shm::Metrics::enabled) {                         \
        if (COND) {                                               \
            redhawk::shm::Metrics::Instance().X.increment(COUNT); \
        }                                                         \
    }

#define RECORD_SHM_METRIC_IF(COND, X) RECORD_SHM_METRIC_ADD_IF(COND, X, 1)
#define RECORD_SHM_METRIC_ADD(X,COUNT) RECORD_SHM_METRIC_ADD_IF(true, X, COUNT)
#define RECORD_SHM_METRIC(X) RECORD_SHM_METRIC_IF(true, X)

#endif // REDHAWK_SHM_METRICS_H
