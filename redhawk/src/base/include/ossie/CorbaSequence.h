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

#ifndef OSSIE_CORBASEQUENCE_H
#define OSSIE_CORBASEQUENCE_H

#include <omniORB4/CORBA.h>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

namespace ossie {

    namespace corba {

        namespace detail {
            // Use the Boost yes and no types for traits templates; yes and no
            // can be distinguished in sizeof() expressions because they are
            // guaranteed to have different sizes.
            using ::boost::type_traits::no_type;
            using ::boost::type_traits::yes_type;

            // Traits class to distinguish whether a given class T is a CORBA
            // sequence or not, for use with enable_if. The class itself is
            // never instantiated. The overloads of the static test() method
            // use SFINAE to check possible type matches that indicate if T is
            // a sequence type based on a couple of heuristics.
            template <class T>
            struct is_sequence {
                // Test for classes derived from _CORBA_Sequence<T> or one of
                // the other sequence types in <omniORB4/seqTemplatedecls.h>;
                // all of these classes typedef themselves as 'T_seq', so if
                // 'T_seq' is a valid type in the given class' scope, it's safe
                // to assume it's a sequence type.
                template <class U>
                static yes_type& test(typename U::T_seq*);

                // Test for classes derived from _CORBA_Sequence_String, which
                // uses a different name for its self-typedef.
                template <class U>
                static yes_type& test(typename U::SeqT*);

                // Default; there is no known type in the class' scope that
                // indicates that it's a CORBA sequence.
                template <class U>
                static no_type& test(...);

                // If type T can be recognized as a sequence, value is true.
                // The test() method isn't actually called, but the compiler
                // evaluates what the return type should be based on the
                // template parameter. The overloads returning a yes type are
                // preferred by the compiler, but will not be considered if
                // they are not valid.
                static const bool value = sizeof(is_sequence::template test<T>(0)) == sizeof(yes_type);
            };

            // Traits class to support generic use of CORBA sequence types via
            // a pointer; the default behavior is to assume that T can be used
            // as a sequence pointer; var types are considered pointers for this
            // purpose. A partial specialization for actual sequence types is
            // is defined later.
            //
            // For most purposes, it can be assumed that a variable of type
            // sequence_ptr::type supports operator->() for calling sequence
            // methods. If T is not valid for usage as a pointer to a sequence,
            // the compiler will flag it as an error.
            //
            // The as_ptr static function converts a T reference into the
            // corresponding pointer type (by default, also a T reference).
            template <class T, class Enable = void>
            struct sequence_ptr {
                typedef T& type;
                typedef const T& const_type;
                static type as_ptr (T& sequence) {
                    return sequence;
                }
            };

            // Partial specialization of sequence pointer traits class for
            // types that can be recognized as CORBA sequences. In this case,
            // we know that T* is a pointer to a sequence, and that converting
            // a T reference into a pointer involves taking the address.
            template <class T>
            struct sequence_ptr<T, typename boost::enable_if<is_sequence<T> >::type> {
                typedef T* type;
                typedef const T* const_type;
                static type as_ptr (T& sequence) {
                    return &sequence;
                }
            };

            // For any type, return the effective "pointer to a sequence." This
            // enables functions to operate generically on sequence types, var
            // types and pointers.
            template <class Sequence>
            inline typename sequence_ptr<Sequence>::type as_ptr (Sequence& sequence)
            {
                return sequence_ptr<Sequence>::as_ptr(sequence);
            }
        }

        // Move the underlying buffer from source to dest, where both are CORBA
        // sequences. Both sequences may be passed by reference or pointer, or
        // as var types, and do not have to be strictly the same type; just the
        // underlying buffer type has to match. Afterwards, source will be an
        // empty sequence, and any prior contents of dest will be released.
        template <typename Tout, typename Tin>
        inline void move (Tout& dest, Tin& source)
        {
            // NB: Both dest and source are converted to pointers to allow a
            //     single implementation for references, pointers and vars; the
            //     compiler will eliminate these temporaries, so there is no
            //     additional overhead created.
            typename detail::sequence_ptr<Tout>::type p_dest = detail::as_ptr(dest);
            typename detail::sequence_ptr<Tin>::type p_src = detail::as_ptr(source);
            const CORBA::ULong maximum = p_src->maximum();
            const CORBA::ULong length = p_src->length();
            p_dest->replace(maximum, length, p_src->get_buffer(1), 1);
        }

        // Append a value to the back of a CORBA sequence; the sequence may be
        // passed by reference or pointer, or as a var type.
        template <typename Sequence, typename Element>
        inline void push_back (Sequence& sequence, const Element& element)
        {
            // NB: The sequence is converted to a pointer to allow a single
            //     implementation for references, pointers and vars; the
            //     compiler will eliminate this temporary, so there is no
            //     additional overhead created.
            typename detail::sequence_ptr<Sequence>::type p_seq = detail::as_ptr(sequence);
            CORBA::ULong index = p_seq->length();
            p_seq->length(index+1);
            p_seq->operator[](index) = element;
        }

        // Remove an indexed value from a CORBA sequence; the sequence may be
        // passed by reference or pointer, or as a var type.
        template <typename Sequence, typename Element>
        inline void erase (Sequence& sequence, const unsigned int &idx)
        {
            // NB: The sequence is converted to a pointer to allow a single
            //     implementation for references, pointers and vars; the
            //     compiler will eliminate this temporary, so there is no
            //     additional overhead created.
            typename detail::sequence_ptr<Sequence>::type p_seq = detail::as_ptr(sequence);
            for (unsigned int i=idx; i<p_seq->length()-1; i++) {
                p_seq->operator[](i) = p_seq->operator[](i+1);
            }
            p_seq->length(p_seq->length()-1);
        }

        // Returns a new sequence containing the range [first, last) from 
        // source; source must be passed by reference, not a var
        template <typename Sequence>
        inline Sequence slice (const Sequence& source, size_t first, size_t last)
        {
            Sequence result;
            const size_t count = last - first;
            result.length(count);
            for (size_t index = 0; first < last; ++first, ++index) {
                result[index] = source[first];
            }
            return result;
        }

        // Returns a new sequence containing the range from first to the end of
        // source; source must be passed by reference, not a var
        template <typename Sequence>
        inline Sequence slice (const Sequence& source, size_t first)
        {
            return slice(source, first, source.length());
        }

        // Appends the values from source to the end of dest; the sequence may
        // be passed by reference or pointer, or as a var type.
        template <class Sequence1, class Sequence2>
        inline void extend(Sequence1& dest, const Sequence2& source)
        {
            // NB: Both dest and source are converted to pointers to allow a
            //     single implementation for references, pointers and vars; the
            //     compiler will eliminate these temporaries, so there is no
            //     additional overhead created.
            typename detail::sequence_ptr<Sequence1>::type p_dest = detail::as_ptr(dest);
            typename detail::sequence_ptr<Sequence2>::const_type p_src = detail::as_ptr(source);
            CORBA::ULong offset = p_dest->length();
            CORBA::ULong length = offset + p_src->length();
            p_dest->length(length);
            for (CORBA::ULong ii = 0; offset < length; ++offset, ++ii) {
                // NB: Var types do not offer a const version of operator[],
                //     but we can get the sequence's const operator[] by
                //     calling it explicitly via the arrow operator
                dest[offset] = p_src->operator[](ii);
            }
        }

    } // namespace corba

} // namespace ossie

#endif // OSSIE_CORBASEQUENCE_H
