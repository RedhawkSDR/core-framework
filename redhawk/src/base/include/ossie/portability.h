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


#ifndef PORTABILITY_H
#define PORTABILITY_H

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

namespace ossie
{
    /**
       Sleep for integer number of seconds.
    */
    inline unsigned int sleep(unsigned int seconds)
    {
#ifdef HAVE_UNISTD_H
        return ::sleep(seconds);
#endif
    }

    /**
       Nano-second resolution sleep.
    */
    inline int nsleep(unsigned long seconds, unsigned long nano_seconds)
    {
#ifdef HAVE_TIME_H
        struct timespec t, rem;

        t.tv_sec = seconds;
        t.tv_nsec = nano_seconds;

        int result = nanosleep(&t, &rem);

        return result;
#endif
    }
}

#endif
