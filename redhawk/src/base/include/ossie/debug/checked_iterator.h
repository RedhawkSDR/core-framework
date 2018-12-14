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

/**
 * @file ossie/debug/checked_iterator.h
 *
 * Checked iterator classes for conditionally enabling additional run-time
 * checking, and to an extent stricter compile-time checking.
 *
 * Inspired by GNU's C++ safe_iterator extensions, but limited in scope to
 * avoid imposing any binary changes on the sequences. This allows users to
 * selectively enable this feature in a compilation unit without affecting
 * other code that may have been compiled with the feature disabled.
 *
 * Checked iterators are intended for development-time use only and should
 * never be used in production systems.
 */

#ifndef REDHAWK_DEBUG_CHECKED_ITERATOR_H
#define REDHAWK_DEBUG_CHECKED_ITERATOR_H

#include <iterator>

#include "check.h"

namespace redhawk {

    namespace debug {

        /**
         * @brief  Base checked iterator wrapper.
         *
         * Aids in comparing const and non-const iterators, providing a common
         * base class and a method to verify both point to the same sequence.
         */
        template <class Sequence>
        class checked_iterator_base {
        public:

            typedef Sequence sequence_type;

            /**
             * @brief  Checks if this iterator can be compared to another.
             * @param other  Iterator to compare against.
             * @return  true if the iterators can be compared.
             *
             * Checked iterators are considered comparable if they belong to
             * the same sequence.
             */
            bool _M_can_compare(const checked_iterator_base& other) const
            {
                return _M_sequence == other._M_sequence;
            }

        protected:
            checked_iterator_base(const sequence_type* sequence) :
                _M_sequence(sequence)
            {
            }

            // Owning sequence
            const sequence_type* _M_sequence;
        };

        /**
         * @brief  Checked iterator wrapper.
         *
         * Iterator wrapper class that performs runtime validity checks on the
         * underlying iterator instance.
         */
        template <class Iterator, class Sequence>
        class checked_iterator : public checked_iterator_base<Sequence> {

            typedef std::iterator_traits<Iterator> _Traits;

        public:
            typedef Iterator iterator_type;
            typedef Sequence sequence_type;
            typedef typename _Traits::difference_type difference_type;
            typedef typename _Traits::value_type value_type;
            typedef typename _Traits::reference reference;
            typedef typename _Traits::pointer pointer;
            typedef typename _Traits::iterator_category iterator_category;

            /**
             * @brief  Create a %checked_iterator.
             * @param current  Underlying iterator.
             * @param sequence  Containing sequence.
             */
            checked_iterator(iterator_type current, const sequence_type* sequence) :
                checked_iterator_base<Sequence>(sequence),
                _M_current(current)
            {
            }

            /**
             * @brief  Copy constructor from a mutable iterator to a constant
             * iterator.
             *
             * If used in the opposite direction (copy from const to mutable),
             * the initialization of _M_current will fail. This is intentional,
             * and easier than conditionally disabling this overload.
             */
            template <typename MutableIterator>
            checked_iterator(const checked_iterator<MutableIterator,Sequence>& other):
                checked_iterator_base<Sequence>(other),
                _M_current(other.base())
            {
            }

            /**
             * @brief  Iterator dereference.
             * @pre iterator is dereferenceable.
             */
            reference operator*() const
            {
                _RH_DEBUG_CHECK(_M_can_dereference());
                return *_M_current;
            }

            /**
             * @brief  Iterator dereference.
             * @pre iterator is dereferenceable.
             */
            pointer operator->() const
            {
                _RH_DEBUG_CHECK(_M_can_dereference());
                return &*_M_current;
            }

            /**
             * @brief  Iterator pre-increment.
             * @pre iterator is incrementable.
             */
            checked_iterator operator++(int)
            {
                // Can't increment past the end
                _RH_DEBUG_CHECK(!_M_is_end());
                checked_iterator tmp(*this);
                ++_M_current;
                return tmp;
            }

            /**
             * @brief  Iterator post-increment.
             * @pre iterator is incrementable.
             */
            checked_iterator& operator++()
            {
                // Can't increment past the end
                _RH_DEBUG_CHECK(!_M_is_end());
                ++_M_current;
                return *this;
            }

            /**
             * @brief  Iterator pre-decrement.
             * @pre iterator is decrementable.
             */
            checked_iterator operator--(int)
            {
                // Can't decrement past the first element
                _RH_DEBUG_CHECK(!_M_is_begin());
                checked_iterator tmp(*this);
                --_M_current;
                return tmp;
            }

            /**
             * @brief  Iterator post-decrement.
             * @pre iterator is decrementable.
             */
            checked_iterator& operator--()
            {
                // Can't decrement past the first element
                _RH_DEBUG_CHECK(!_M_is_begin());
                --_M_current;
                return *this;
            }

            /**
             * @brief  Indexed iterator dereference.
             * @param index  Index of element to dereference.
             * @pre iterator plus index does not exceed bounds of sequence.
             */
            reference operator[](const difference_type& index) const
            {
                _RH_DEBUG_CHECK(_M_can_increment(index+1));
                return _M_current[index];
            }

            /**
             * @brief  Iterator in-place add.
             * @param offset  Number of elements to advance.
             * @pre iterator plus offset does not go past the sequence's end().
             */
            checked_iterator& operator+=(const difference_type& offset)
            {
                _RH_DEBUG_CHECK(_M_can_increment(offset));
                _M_current += offset;
                return *this;
            }

            /**
             * @brief  Iterator add.
             * @param offset  Number of elements to advance.
             * @return  Iterator advanced by offset elements.
             * @pre iterator plus offset does not go past the sequence's end().
             */
            checked_iterator operator+(const difference_type& offset) const
            {
                checked_iterator result(*this);
                result += offset;
                return result;
            }

            /**
             * @brief  Iterator in-place subtract.
             * @param offset  Number of elements to reverse.
             * @pre iterator minus offset does not go past the sequence's
             * begin().
             */
            checked_iterator& operator-=(const difference_type& offset)
            {
                _RH_DEBUG_CHECK(_M_can_decrement(offset));
                _M_current -= offset;
                return *this;
            }

            /**
             * @brief  Iterator subtract.
             * @param offset  Number of elements to reverse.
             * @return  Iterator moved backwards by offset elements.
             * @pre iterator minus offset does not go past the sequence's
             * begin().
             */
            checked_iterator operator-(const difference_type& offset) const
            {
                checked_iterator result(*this);
                result -= offset;
                return result;
            }

            /**
             * @brief  Return the underlying iterator.
             */
            iterator_type base() const
            {
                return _M_current;
            }

            /**
             * @brief  Conversion to underlying iterator type.
             *
             * Allows implicit conversions in expressions.
             */
            operator iterator_type() const
            {
                return _M_current;
            }

        private:
            // Predicate to check if this iterator is at the beginning of the
            // sequence
            bool _M_is_begin() const
            {
                return *this == this->_M_sequence->begin();
            }

            // Predicate to check if this iterator is at the end of the
            // sequence
            bool _M_is_end() const
            {
                return *this == this->_M_sequence->end();
            }

            // Predicate to check if this iterator can be dereferenced (i.e.,
            // can use operator*); the only condition that is checked is that
            // it is not at the end, under the assumption that it cannot be
            // outside the range [begin, end), otherwise a prior check would
            // have failed
            bool _M_can_dereference() const
            {
                return !_M_is_end();
            }

            // Checks whether this iterator can be incremented by the given
            // number of elements
            bool _M_can_increment(int count) const
            {
                typedef typename Sequence::const_iterator const_iterator_type;
                return std::distance<const_iterator_type>(*this, this->_M_sequence->end()) >= count;
            }

            // Checks whether this iterator can be decremented by the given
            // number of elements
            bool _M_can_decrement(int count) const
            {
                typedef typename Sequence::const_iterator const_iterator_type;
                return std::distance<const_iterator_type>(this->_M_sequence->begin(), this) >= count;
            }

            // Underlying iterator value
            iterator_type _M_current;
        };

        // The operators below are templatized for two iterator types instead
        // of just one to support operations between const and non-const
        // iterators.

        template <typename I1, typename I2, typename Sequence>
        inline bool operator==(const checked_iterator<I1, Sequence>& lhs,
                               const checked_iterator<I2, Sequence>& rhs)
        {
            _RH_DEBUG_CHECK(lhs._M_can_compare(rhs));
            return lhs.base() == rhs.base();
        }

        template <typename I1, typename I2, typename Sequence>
        inline bool operator!=(const checked_iterator<I1, Sequence>& lhs,
                               const checked_iterator<I2, Sequence>& rhs)
        {
            _RH_DEBUG_CHECK(lhs._M_can_compare(rhs));
            return lhs.base() != rhs.base();
        }

        template <typename I1, typename I2, typename Sequence>
        inline bool operator<(const checked_iterator<I1, Sequence>& lhs,
                              const checked_iterator<I2, Sequence>& rhs)
        {
            _RH_DEBUG_CHECK(lhs._M_can_compare(rhs));
            return lhs.base() < rhs.base();
        }

        template <typename I1, typename I2, typename Sequence>
        inline bool operator<=(const checked_iterator<I1, Sequence>& lhs,
                               const checked_iterator<I2, Sequence>& rhs)
        {
            _RH_DEBUG_CHECK(lhs._M_can_compare(rhs));
            return lhs.base() <= rhs.base();
        }

        template <typename I1, typename I2, typename Sequence>
        inline bool operator>(const checked_iterator<I1, Sequence>& lhs,
                              const checked_iterator<I2, Sequence>& rhs)
        {
            _RH_DEBUG_CHECK(lhs._M_can_compare(rhs));
            return lhs.base() > rhs.base();
        }

        template <typename I1, typename I2, typename Sequence>
        inline bool operator>=(const checked_iterator<I1, Sequence>& lhs,
                               const checked_iterator<I2, Sequence>& rhs)
        {
            _RH_DEBUG_CHECK(lhs._M_can_compare(rhs));
            return lhs.base() >= rhs.base();
        }

        template <typename I1, typename I2, typename Sequence>
        inline typename checked_iterator<I1, Sequence>::difference_type
        operator-(const checked_iterator<I1, Sequence>& lhs,
                  const checked_iterator<I2, Sequence>& rhs)
        {
            _RH_DEBUG_CHECK(lhs._M_can_compare(rhs));
            return lhs.base() - rhs.base();
        }

    } // namespace debug

} // namespace redhawk

#endif // _CHECKED_ITERATOR_HH_
