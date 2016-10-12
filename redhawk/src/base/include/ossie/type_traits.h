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

#ifndef OSSIE_TYPE_TRAITS_H
#define OSSIE_TYPE_TRAITS_H

#include <string>
#include <complex>
#include <vector>
#include <typeinfo>

#include <omniORB4/CORBA.h>

namespace ossie {

    namespace internal {
	const std::string demangle (const std::string&);
    }

    template <typename T>
    struct traits
    {
        // Default implementation: try to demangle the type name as returned by
        // the typeid operator
	static inline const std::string name()
	{
	    return ossie::internal::demangle(typeid(T).name());
	}
    };

    // Specializations for explicitly-sized types to avoid confusion about the
    // correct type; if, e.g., a 32-bit integer type is required, it should
    // return the same type name regardless of the architecture (long is 32
    // bits on x86, but 64 on x86_64)
    template <>
    inline const std::string traits<CORBA::Long>::name ()
    {
	return "CORBA::Long";
    }

    template <>
    inline const std::string traits<CORBA::ULong>::name ()
    {
	return "CORBA::ULong";
    }

    template <>
    inline const std::string traits<CORBA::LongLong>::name ()
    {
	return "CORBA::LongLong";
    }

    template <>
    inline const std::string traits<CORBA::ULongLong>::name ()
    {
	return "CORBA::ULongLong";
    }

    // Specializations for other common types, to ensure that a meaningful name
    // is returned if the demangling API is unavailable
    template <>
    inline const std::string traits<char>::name ()
    {
	return "char";
    }

    template <>
    inline const std::string traits<signed char>::name ()
    {
	return "signed char";
    }

    template <>
    inline const std::string traits<unsigned char>::name ()
    {
	return "unsigned char";
    }

    template <>
    inline const std::string traits<bool>::name ()
    {
	return "bool";
    }

    template <>
    inline const std::string traits<std::string>::name ()
    {
	return "std::string";
    }

    template <>
    inline const std::string traits<short>::name ()
    {
	return "short";
    }

    template <>
    inline const std::string traits<unsigned short>::name ()
    {
	return "unsigned short";
    }

    template <>
    inline const std::string traits<float>::name ()
    {
	return "float";
    }

    template <>
    inline const std::string traits<double>::name ()
    {
	return "double";
    }

#if SIZEOF_LONG == 8
    // Specialization for 64-bit machines--even though both "long" and
    // "long long" are 64 bits, they are distinct types
    template <>
    inline const std::string traits<long long>::name ()
    {
	return "long long";
    }

    template <>
    inline const std::string traits<unsigned long long>::name ()
    {
	return "unsigned long long";
    }
#endif

    // Specializations for common template classes to ensure that primitive
    // types follow the naming above
    template <typename T>
    struct traits<std::complex<T> >
    {
	static inline const std::string name ()
	{
	    return "std::complex<" + traits<T>::name() + ">";
	}
    };

    template <typename T>
    struct traits<std::vector<T> >
    {
	static inline const std::string name ()
	{
	    return "std::vector<" + traits<T>::name() + ">";
	}
    };

    // Specializations for CV-qualified types; may be combined with other
    // modifiers, but "const volatile" is a distinct specialization to resolve
    // ambiguities
    template <typename T>
    struct traits<const T>
    {
        static inline const std::string name ()
        {
            return traits<T>::name() + " const";
        }
    };

    template <typename T>
    struct traits<volatile T>
    {
        static inline const std::string name ()
        {
            return traits<T>::name() + " volatile";
        }
    };

    template <typename T>
    struct traits<const volatile T>
    {
        static inline const std::string name ()
        {
            return  traits<T>::name() + " const volatile";
        }
    };

    // Specialization for pointer types; may be combined with other modifiers
    template <typename T>
    struct traits<T*>
    {
        static inline const std::string name ()
        {
            return traits<T>::name() + "*";
        }
    };

    // Specialization for reference types; may be combined with other modifiers
    template <typename T>
    struct traits<T&>
    {
        static inline const std::string name ()
        {
            return traits<T>::name() + "&";
        }
    };
}

#endif // OSSIE_TYPE_TRAITS_H
