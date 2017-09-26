#ifndef THREADSTATE_HH
#define THREADSTATE_HH

namespace redhawk {
    class ShmArena;

    class ThreadState {
    public:
        ThreadState() :
            last(0),
            contention(0)
        {
        }

        ShmArena* last;
        int contention;
    };
}

#endif // THREADSTATE_HH
