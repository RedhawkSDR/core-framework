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
#ifndef REFERENCE_WRAPPER_H_
#define REFERENCE_WRAPPER_H_

/**
 * ReferenceWrapper is a minimal backport of the std::reference_wrapper class
 * from C++11.  It acts as a function object by providing the () operator
 * (unlike boost::reference_wrapper).
 */
template< class REFERENCE_TYPE, class RETURN_TYPE=REFERENCE_TYPE >
class ReferenceWrapper
{
public:
	typedef REFERENCE_TYPE type;
	typedef RETURN_TYPE result_type;
	typedef void argument_type;

	explicit ReferenceWrapper( type& ref ): ref_(ref){}

	result_type operator()() const { return static_cast<result_type>(ref_); }

	type& get() const { return ref_; }

private:
	type& ref_;
};


template< class REFERENCE_TYPE >
ReferenceWrapper<REFERENCE_TYPE> MakeRef( REFERENCE_TYPE& ref )
{
	return ReferenceWrapper<REFERENCE_TYPE>(ref);
}

template< class REFERENCE_TYPE, class RETURN_TYPE >
ReferenceWrapper<REFERENCE_TYPE, RETURN_TYPE> MakeRef( REFERENCE_TYPE const& ref )
{
	return ReferenceWrapper<REFERENCE_TYPE, RETURN_TYPE>(ref);
}

template< class REFERENCE_TYPE >
ReferenceWrapper<REFERENCE_TYPE const> MakeCref( REFERENCE_TYPE const& ref )
{
	//std::cout << "MakeCref: addr=" << (size_t)((REFERENCE_TYPE const*)(&ref)) << std::endl;
	return ReferenceWrapper<REFERENCE_TYPE const>(ref);
}

template< class REFERENCE_TYPE, class RETURN_TYPE >
ReferenceWrapper<REFERENCE_TYPE const, RETURN_TYPE const> MakeCref( REFERENCE_TYPE const& ref )
{
	return ReferenceWrapper<REFERENCE_TYPE const, RETURN_TYPE const>(ref);
}


#endif
