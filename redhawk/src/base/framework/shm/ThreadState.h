#ifndef REDHAWK_THREADSTATE_H
#define REDHAWK_THREADSTATE_H

namespace redhawk {

    namespace shm {
        class Superblock;

        class ThreadState {
        public:
            ThreadState() :
                last(0),
                contention(0)
            {
            }

            shm::Superblock* last;
            int contention;
        };
    }
}

#endif // REDHAWK_THREADSTATE_H
