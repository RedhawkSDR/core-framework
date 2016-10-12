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

#ifndef OSSIE_GCTHREAD_H
#define OSSIE_GCTHREAD_H

#include <list>
#include <boost/thread.hpp>

namespace ossie {
    class GCContext;

    class GCThread
    {
    public:
        static void add(GCContext* context);
        static void release(GCContext* context);

        static void shutdown();

    private:
        GCThread();
        ~GCThread();

        static GCThread* instance();
        static boost::mutex instanceMutex_;
        static GCThread* instance_;

        void addContext(GCContext* context);
        void releaseContext(GCContext* context);

        void sweep();
        void run();
        void stop();

        boost::mutex contextsMutex_;
        std::list<GCContext*> contexts_;

        boost::posix_time::time_duration sleepTime_;

        boost::thread thread_;
    };
}

#endif // GCTHREAD_H
