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
	static inline const std::string name()
	{
	    return ossie::internal::demangle(typeid(T).name());
	}
    };

    template <>
    inline const std::string traits<char>::name ()
    {
	return "char";
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

}

#endif // OSSIE_TYPE_TRAITS_H
