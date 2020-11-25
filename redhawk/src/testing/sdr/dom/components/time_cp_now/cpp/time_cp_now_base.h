#ifndef TIME_CP_NOW_BASE_IMPL_BASE_H
#define TIME_CP_NOW_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class time_cp_now_base : public Component, protected ThreadedComponent
{
    public:
        time_cp_now_base(const char *uuid, const char *label);
        ~time_cp_now_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: rightnow
        CF::UTCTime rightnow;
        /// Property: simple1970
        CF::UTCTime simple1970;
        /// Property: simpleSeqDefNow
        std::vector<CF::UTCTime> simpleSeqDefNow;
        /// Property: simpleSeqNoDef
        std::vector<CF::UTCTime> simpleSeqNoDef;
        /// Property: simpleSeq1970
        std::vector<CF::UTCTime> simpleSeq1970;

    private:
};
#endif // TIME_CP_NOW_BASE_IMPL_BASE_H
