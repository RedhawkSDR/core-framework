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

namespace redhawk {

    template <class Node, Node* Node::*Prev=&Node::prev, Node* Node::*Next=&Node::next>
    struct list_node_traits {
        typedef Node node_type;

        static inline void set_prev(node_type& node, node_type* prev)
        {
            node.*Prev = prev;
        }

        static inline node_type* get_prev(const node_type& node)
        {
            return node.*Prev;
        }

        static inline void set_next(node_type& node, node_type* next)
        {
            node.*Next = next;
        }

        static inline node_type* get_next(const node_type& node)
        {
            return node.*Next;
        }
    };

    template <class Node, class NodeTraits>
    struct list_iterator {
    public:
        typedef NodeTraits node_traits;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef typename NodeTraits::node_type value_type;
        typedef ptrdiff_t difference_type;
        typedef Node* pointer;
        typedef Node& reference;

        list_iterator(pointer node=0) :
            _M_node(node)
        {
        }

        list_iterator(const list_iterator<value_type,node_traits>& other) :
            _M_node(other.get_node())
        {
        }

        list_iterator& operator++()
        {
            _M_node = node_traits::get_next(*_M_node);
            return *this;
        }

        list_iterator operator++(int)
        {
            list_iterator result(*this);
            return ++result;
        }

        list_iterator& operator--()
        {
            _M_node = node_traits::get_prev(*_M_node);
            return *this;
        }

        list_iterator operator--(int)
        {
            list_iterator result(*this);
            return --result;
        }

        reference operator*() const
        {
            return *get_node();
        }

        pointer operator->() const
        {
            return get_node();
        }

        pointer get_node() const
        {
            return _M_node;
        }

        bool operator==(const list_iterator& other) const
        {
            return (_M_node == other._M_node);
        }

        bool operator!=(const list_iterator& other) const
        {
            return !(*this == other);
        }

    private:
        pointer _M_node;
    };

    template <class Node, class NodeTraits=list_node_traits<Node> >
    class inplace_list {
    public:
        typedef Node node_type;
        typedef NodeTraits node_traits;

        typedef list_iterator<node_type, node_traits> iterator;
        typedef list_iterator<const node_type, node_traits> const_iterator;

        inplace_list() :
            _M_head(0),
            _M_tail(0),
            _M_size(0)
        {
        }

        size_t size() const
        {
            return _M_size;
        }

        bool empty() const
        {
            return (_M_head == 0);
        }

        iterator begin()
        {
            return iterator(_M_head);
        }

        iterator end()
        {
            return iterator();
        }

        const_iterator begin() const
        {
            return const_iterator(_M_head);
        }

        const_iterator end() const
        {
            return const_iterator();
        }

        void insert(iterator pos, node_type& node)
        {
            node_type* prev = 0;
            node_type* next = 0;
            if (pos == begin()) {
                next = _M_head;
                _M_head = &node;
                if (!_M_tail) {
                    _M_tail = _M_head;
                }
            } else if (pos == end()) {   
                prev = _M_tail;
                _M_tail = &node;
            } else {
                prev = node_traits::get_prev(*pos);
                next = &(*pos);
            }

            if (prev) {
                node_traits::set_next(*prev, &node);
            }
            node_traits::set_prev(node, prev);
            node_traits::set_next(node, next);
            if (next) {
                node_traits::set_prev(*next, &node);
            }
            ++_M_size;
        }

        void erase(iterator iter)
        {
            node_type* prev = node_traits::get_prev(*iter);
            node_type* next = node_traits::get_next(*iter);
            if (!prev) {
                _M_head = next;
            } else {
                node_traits::set_next(*prev, next);
            }
            if (next) {
                node_traits::set_prev(*next, prev);
            } else {
                _M_tail = prev;
            }
            --_M_size;
        }

        void push_front(node_type& node)
        {
            insert(begin(), node);
        }

        node_type& front()
        {
            return *_M_head;
        }

        void pop_back()
        {
            erase(_M_tail);
        }

        node_type& back()
        {
            return *_M_tail;
        }

    private:
        node_type* _M_head;
        node_type* _M_tail;
        size_t _M_size;
    };

}
