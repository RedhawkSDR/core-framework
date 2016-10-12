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

#ifndef __OSSIEPARSER_H__
#define __OSSIEPARSER_H__

#include <iostream>
#include <memory>
#include <cassert>
#include <vector>

#define OSSIEPARSER_API

// Provide forward definitions of the internal parser data structures
// The definitions of these are hidden (using the Qpaque Pointer idiom)
// in the libossieparser library.  This allows the underlying parser
// and data structures to be changed without requiring a recompilation.
namespace ossie {

    template<class T>
    class optional_value
    {
        public:
            template< typename charT, typename Traits, typename U>
            friend std::basic_ostream<charT, Traits>& operator<<(std::basic_ostream<charT, Traits> &out, const optional_value<U> ov);

            optional_value() : _p(0) {
            }

            optional_value(const T& v) : _p(0) {
                _p.reset(new T(v));
            }

            optional_value(const T* p) : _p(0) {
                if (p != 0) {
                    _p.reset(new T(*p));
                }
            }

            optional_value(const optional_value<T>& ov) {
                if (ov._p.get() != 0) {
                    _p.reset(new T(*(ov._p)));
                    assert(_p.get() != 0);
                }
            }

            bool isSet() const {
                return (_p.get() != 0);
            }

            const T* get() const {
                return _p.get();
            }

            T& operator*() const {
                assert(_p.get() != 0);
                return *(_p.get());
            }

            T* operator->() const {
                return _p.get();
            }

            optional_value<T>& operator=(const T& v) {
                _p.reset(new T(v));
                assert(_p.get() != 0);
                return *this;
            }

            optional_value<T>& operator=(const optional_value<T>& ov) {
                if (ov._p.get() != 0) {
                    _p.reset(new T(*(ov._p)));
                    assert(_p.get() != 0);
                } else {
                    _p.reset(0);
                }
                return *this;
            }

        private:
            std::auto_ptr<T> _p;

    };

    template< typename charT, typename Traits, typename U>
    std::basic_ostream<charT, Traits>& operator<<(std::basic_ostream<charT, Traits> &out, const optional_value<U> ov)
    {
        if (ov.isSet()) {
            out << *(ov._p);
        } else {
            out << "<not set>";
        }
        return out;
    }
}
#endif
