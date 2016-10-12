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

#ifndef COMPLEXPROPERTIES_H
#define COMPLEXPROPERTIES_H

#include "ossie/PropertyInterface.h"
#include "CF/cf.h"

#define ANY_COMPLEX_OPERATORS(T, CFT)                                             \
inline bool operator>>= (const CORBA::Any& _a, std::complex<T>& _c) {             \
    CFT* temp;                                                                    \
    if (!(_a >>= temp)) return false;                                             \
    _c = std::complex<T>(temp->real, temp->imag);                                 \
    return true;                                                                  \
}                                                                                 \
inline void operator<<= (CORBA::Any& _a, const std::complex<T>& _c) {             \
    CFT temp;                                                                     \
    temp.real = _c.real();                                                        \
    temp.imag = _c.imag();                                                        \
    _a <<= temp;                                                                  \
}                                                                                 \
inline bool operator>>= (const CORBA::Any& _a, std::vector<std::complex<T> >& _s) \
{                                                                                 \
    return ossie::corba::vector_extract<std::complex<T>, CFT##Seq>(_a, _s);       \
}                                                                                 \
inline void operator<<= (CORBA::Any& _a, const std::vector<std::complex<T> >& _s) \
{                                                                                 \
    ossie::corba::vector_insert<std::complex<T>, CFT##Seq, CFT>(_a, _s);          \
}

ANY_COMPLEX_OPERATORS(bool, CF::complexBoolean);
ANY_COMPLEX_OPERATORS(short, CF::complexShort);
ANY_COMPLEX_OPERATORS(float, CF::complexFloat);
ANY_COMPLEX_OPERATORS(double, CF::complexDouble);
ANY_COMPLEX_OPERATORS(unsigned char, CF::complexOctet);
ANY_COMPLEX_OPERATORS(char, CF::complexChar);
ANY_COMPLEX_OPERATORS(CORBA::Long, CF::complexLong);
ANY_COMPLEX_OPERATORS(CORBA::ULong, CF::complexULong);
ANY_COMPLEX_OPERATORS(CORBA::LongLong, CF::complexLongLong);
ANY_COMPLEX_OPERATORS(CORBA::ULongLong, CF::complexULongLong);
ANY_COMPLEX_OPERATORS(unsigned short, CF::complexUShort);

#undef ANY_COMPLEX_OPERATORS

#endif // PROPERTYINTERFACE_H
