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

#ifndef ATOMIC_COUNTER_H
#define ATOMIC_COUNTER_H

template <typename T>
class atomic_counter {
public:
    typedef T counter_type;

    atomic_counter() :
        _value(0)
    {
    }

    explicit atomic_counter(counter_type value) :
        _value(value)
    {
    }

    operator counter_type() const
    {
#ifdef __ATOMIC_RELAXED
        counter_type result;
        __atomic_load(&_value, &result, __ATOMIC_RELAXED);
        return result;
#else
        return _value;
#endif
    }

    atomic_counter& operator=(counter_type value)
    {
#ifdef __ATOMIC_RELAXED
        __atomic_store(&_value, &value, __ATOMIC_RELAXED);
#else
        _value = value;
#endif
        return *this;
    }

    counter_type increment()
    {
#ifdef __ATOMIC_RELAXED
        return __atomic_add_fetch(&_value, 1, __ATOMIC_RELAXED);
#else
        return __sync_add_and_fetch(&_value, 1);
#endif
    }

    counter_type decrement()
    {
#ifdef __ATOMIC_RELAXED
        return __atomic_sub_fetch(&_value, 1, __ATOMIC_RELAXED);
#else
        return __sync_sub_and_fetch(&_value, 1);
#endif        
    }

private:
    volatile counter_type _value;
};

#endif // ATOMIC_COUNTER_H
