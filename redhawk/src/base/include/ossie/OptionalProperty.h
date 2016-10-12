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

#ifndef __OPTIONALPROPERTY_H__
#define __OPTIONALPROPERTY_H__

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

template<class T>
class optional_property
{
    public:
        template< typename charT, typename Traits, typename U>
        friend std::basic_ostream<charT, Traits>& operator<<(std::basic_ostream<charT, Traits> &out, const optional_property<U> ov);

        optional_property() : _p(0) {
        }

        optional_property(const T& v) : _p(0) {
            _p.reset(new T(v));
        }

        optional_property(const T* p) : _p(0) {
            if (p != 0) {
                _p.reset(new T(*p));
            }
        }

        optional_property(const optional_property<T>& ov) {
            if (ov._p.get() != 0) {
                _p.reset(new T(*(ov._p)));
            }
        }

        bool isSet() const {
            return (_p.get() != 0);
        }

        const T* get() const {
            return _p.get();
        }

        T& operator*() const throw (std::runtime_error) {
            if (_p.get() == 0) {
                throw std::runtime_error("Attempted to use unset optional property.");
            }
            return *(_p.get());
        }

        T* operator->() const {
            return _p.get();
        }

        optional_property<T>& operator=(const T& v) {
            _p.reset(new T(v));
            return *this;
        }

        optional_property<T>& operator=(const optional_property<T>& ov) {
            if (ov._p.get() != 0) {
                _p.reset(new T(*(ov._p)));
            } else {
                _p.reset(0);
            }
            return *this;
        }

        void reset() {
            _p.reset(0);
        }

    private:
        std::auto_ptr<T> _p;

};

template<class T>
inline bool operator==(const optional_property<T>& lhs, const optional_property<T>& rhs)
{
    if (lhs.get() != 0 && rhs.get() != 0) {
        if (*lhs == *rhs) {
            return true;
        }
    } 
    return false;   
}

template<class T>
inline bool operator!=(const optional_property<T>& lhs, const optional_property<T>& rhs)
{
    return !(lhs == rhs);
}


template< typename charT, typename Traits, typename U>
std::basic_ostream<charT, Traits>& operator<<(std::basic_ostream<charT, Traits> &out, const optional_property<U> ov)
{
    if (ov.isSet()) {
        out << *(ov._p);
    } else {
        out << "<not set>";
    }
    return out;
}

#endif
