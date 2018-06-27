/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef CONV_WRAPPER_H_
#define CONV_WRAPPER_H_
#include <stdint.h>
#include <functional>
template< class REFERENCE_TYPE, class RETURN_TYPE=REFERENCE_TYPE, class ctype=uint64_t,  class CFUNC=std::multiplies< RETURN_TYPE >   >
class ConversionWrapper
  {
  public:
  typedef REFERENCE_TYPE type;
  typedef RETURN_TYPE result_type;
  typedef void argument_type;
  typedef CFUNC      opfunc;

  explicit ConversionWrapper( type& ref, ctype cf=1048576, const opfunc &func=std::multiplies< result_type >() ):
  ref_(ref), func_(func), unit_conversion_(cf)
  {};

  result_type operator()() const {
    return  func_( static_cast<result_type>(ref_), (result_type)unit_conversion_ );
    // debug
#if 0
    result_type ret;
    ret = func_( static_cast<result_type>(ref_), (result_type)unit_conversion_ );
    std::cout << " ConversionWrapper:  value/cf/result " << ref_ << "/" << unit_conversion_ << "/" << ret << std::endl;
    return ret;
#endif
  };

  type& get() const { return ref_; };

  private:
  type  &ref_;
  opfunc  func_;
  ctype  unit_conversion_;

  };

#endif
