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

#ifndef OSSIE_CALLBACK_H
#define OSSIE_CALLBACK_H

#include <list>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "internal/equals.h"

namespace redhawk {
    namespace detail {
        // Bring ossie::internal::has_equals into this namespace (in effect) so
        // we can specialize it for boost::bind
        template <class T>
        struct has_equals : public ossie::internal::has_equals<T> { };

        // Special case: boost::bind returns objects whose operator== returns
        // another bind object, as opposed to a boolean, which breaks the
        // default has_equals template. However, it does have one, and can be
        // compared for equality using function_equal.
        template <class R, class F, class L>
        struct has_equals<boost::_bi::bind_t<R,F,L> > {
            static bool const value = true;
        };

        // Helper class to manage an object via void pointers, to avoid
        // generating a lot of virtual functions and object type information
        // when using callbacks. The object is stored in a buffer directly
        // following the void_manager; its type is erased, but the key
        // functions of copy/delete and equality comparison are maintained as
        // function pointers that are able to cast back to the original object
        // type.
        struct void_manager {
            ~void_manager()
            {
                _M_deleter(get_object());
            }

            // Returns a pointer to the managed object
            inline void* get_object()
            {
                return &_M_buf[0];
            }

            // Returns a const pointer to the managed object
            inline const void* get_object() const
            {
                return &_M_buf[0];
            }

            // Copies this void_manager and its contained object
            void_manager* clone() const
            {
                // Allocate enough space for the manager plus the object, then
                // initialize the manager with placement new
                void* buf = _M_allocate(_M_size);
                void_manager* manager = new (buf) void_manager(*this);

                // Dispatch to the object copy constructor (via _M_clone)
                _M_clone(manager->get_object(), get_object());
                return manager;
            }

            // Compares this void_manager's contained object with another's.
            bool operator==(const void_manager& other) const
            {
                // If the object doesn't support equality comparison, consider
                // the objects unequal. If the two managers have different
                // equality function pointers, the types must differ, so also
                // consider the objects unequal; this may be more limiting than
                // the compiler might allow (e.g., due to implict conversions
                // or overloads), but is the safest approach without more type
                // information.
                if (!_M_equal || (_M_equal != other._M_equal)) {
                    return false;
                }
                return _M_equal(get_object(), other.get_object());
            }

            // Create a new void_manager with a copy of the object
            template <class T>
            static void_manager* create(const T& object)
            {
                // Allocate enough space for the void_manager and the object,
                // then initialize with placement new
                void* buf = _M_allocate(sizeof(T));
                void_manager* manager = new (buf) void_manager(&deleter<T>, &clone<T>, get_equals<T>(), sizeof(T));

                // Clone the input object into the new void_manager
                manager->_M_clone(manager->get_object(), &object);
                return manager;
            }

        private:
            // Function types for object management; these adapt the object
            // type, which is only known at creation time, to void pointers
            typedef void (*deleter_func)(void*);
            typedef void (*clone_func)(void* dest, const void* src);
            typedef bool (*equal_func)(const void*, const void*);

            // Create a new void manager; can only be used from create()
            void_manager(deleter_func deleter, clone_func clone, equal_func equal, size_t size) :
                _M_deleter(deleter),
                _M_clone(clone),
                _M_equal(equal),
                _M_size(size)
            {
            }

            // Allocates enough memory for a void_manager with an object buffer
            // of the given size
            static void* _M_allocate(size_t size)
            {
                size_t bytes = sizeof(void_manager) + size - sizeof(_M_buf);
                return ::operator new(bytes);
            }

            // Deleter function template, instantiated by create (as a function
            // pointer)
            template <class T>
            static void deleter(void* data)
            {
                // Call the stored object's destructor directly instead of
                // delete, because the memory is part of the void_manager
                // instance
                static_cast<T*>(data)->~T();
            }

            // Copy function template, instantiated by create (as a function
            // pointer)
            template <class T>
            static void clone(void* dest, const void* src)
            {
                // Call the placement new copy constructor into the destination
                // object buffer; the memory itself is managed by the
                // void_manager instance
                new (dest) T(*static_cast<const T*>(src));
            }

            // Equals function template, instantiated by create (as a function
            // pointer), but only if the template type supports testing for
            // equality
            template <class T>
            static bool equals(const void* lhs, const void* rhs)
            {
                return equals(*static_cast<const T*>(lhs), *static_cast<const T*>(rhs));
            }

            // Implementation of equality function for types that support it
            // normally
            template <typename T>
            static inline bool equals(const T& lhs, const T& rhs)
            {
                return lhs == rhs;
            }

            // Overload of equality function for boost::bind objects, which are
            // not possible to compare with operator==
            template <class R, class F, class L>
            static inline bool equals(const boost::_bi::bind_t<R,F,L>& lhs, const boost::_bi::bind_t<R,F,L>& rhs)
            {
                return function_equal(lhs, rhs);
            }

            // Template function to get the function pointer for equals, for
            // types that cannot be compared for equality. Unlike the other
            // function pointers, equals is looked up in this way to avoid
            // instantiating the function template unless it's needed, because
            // when used as a function pointer it creates a linker symbol.
            template <class T>
            static inline equal_func get_equals(typename boost::disable_if<has_equals<T> >::type* unused=0)
            {
                return 0;
            }

            // Template function to get the function pointer for equals, for
            // types that can be compared for equality
            template <class T>
            static inline equal_func get_equals(typename boost::enable_if<has_equals<T> >::type* unused=0)
            {
                return &equals<T>;
            }

            // Object management function pointers; see type definitions above
            deleter_func _M_deleter;
            clone_func _M_clone;
            equal_func _M_equal;

            // The size of the managed object
            size_t _M_size;

            // Buffer for storing the managed object; its true size is known at
            // allocation time (and stored in _M_size), but it must be at least
            // 1. The object storage starts at _M_buf[0] and goes to the end of
            // the allocated memory.
            char _M_buf[1];
        };

        // Forward declaration of an "unusable" argument type. For simplicity
        // of implementation, callback provides zero, one and two argument
        // versions of operator(); without variadic templates, we would have to
        // create specializations for each argument count. The overloads with
        // too many arguments take an "unusable" struct, which will never match
        // the argument type, giving an error message.
        struct unusable;

        // Traits class to bind a function signature Sig to an invoker function
        // type. Callback invokers take a void pointer as the first argument,
        // which is a type-erased pointer to a callable object (e.g., function
        // pointer, member_function, etc.), followed by the normal arguments.
        // This is how the callback class dispatches function calls.
        //
        // This class must be specialized for each number of arguments (in this
        // case zero, one and two). The result type is also included in the
        // typedef rather than using Boost's function traits classes because
        // it's simple enough to include on our own.
        template <class Sig>
        struct callback_traits;

        // Specialization for zero-argument invoker function type
        template <class R>
        struct callback_traits<R ()> {
            typedef R result_type;
            typedef unusable first_argument_type;
            typedef unusable second_argument_type;
            typedef R (*invoker_func)(void*);
        };

        // Specialization for two-argument invoker function type
        template <class R, class A1>
        struct callback_traits<R (A1)> {
            typedef R result_type;
            typedef A1 first_argument_type;
            typedef unusable second_argument_type;
            typedef R (*invoker_func)(void*, A1);
        };

        // Specialization for two-argument invoker function type
        template <class R, class A1, class A2>
        struct callback_traits<R (A1,A2)> {
            typedef R result_type;
            typedef A1 first_argument_type;
            typedef A2 second_argument_type;
            typedef R (*invoker_func)(void*, A1, A2);
        };

        // Templatized class that adapts the callable type Func to the function
        // signature Sig. This must be specialized for each number of arguments
        // supported (in this case zero, one and two) with a static function
        // call() that is used as a function pointer; it must be assignable to
        // a callback_traits<Sig>::invoker_func. The call() function receives
        // the callable as a void* (which can then be static cast back to the
        // callable type) and any arguments declared in Sig.
        //
        // For each argument count, this should be further specialized for the
        // void return versions to adapt non-void callables to void return
        // signatures. The compiler allows statements like "return f();" if
        // both functions return void, but if f() returns a value it becomes a
        // compilation error.
        template <class Func, class Sig>
        struct function_invoker;

        // Specialization for zero-argument invocation function
        template <class Func, class R>
        struct function_invoker<Func, R ()> {
            static R call(void* data)
            {
                Func* func = static_cast<Func*>(data);
                return (*func)();
            }
        };

        // Specialization for zero-argument void return invocation function
        template <class Func>
        struct function_invoker<Func, void ()> {
            static void call(void* data)
            {
                Func* func = static_cast<Func*>(data);
                (*func)();
            }
        };

        // Specialization for one-argument invocation function
        template <class Func, class R, class A1>
        struct function_invoker<Func, R (A1)> {
            static R call(void* data, A1 a1)
            {
                Func* func = static_cast<Func*>(data);
                return (*func)(a1);
            }
        };

        // Specialization for one-argument void return invocation function
        template <class Func, class A1>
        struct function_invoker<Func, void (A1)> {
            static void call(void* data, A1 a1)
            {
                Func* func = static_cast<Func*>(data);
                (*func)(a1);
            }
        };

        // Specialization for two-argument invocation function
        template <class Func, class R, class A1, class A2>
        struct function_invoker<Func, R (A1,A2)> {
            static R call(void* data, A1 a1, A2 a2)
            {
                Func* func = static_cast<Func*>(data);
                return (*func)(a1, a2);
            }
        };

        // Specialization for two-argument void return invocation function
        template <class Func, class A1, class A2>
        struct function_invoker<Func, void (A1,A2)> {
            static void call(void* data, A1 a1, A2 a2)
            {
                Func* func = static_cast<Func*>(data);
                (*func)(a1, a2);
            }
        };

        // Template class to bind together a member function with an object
        // instance. The instance may be stored by pointer, value or shared
        // pointer; the function must be a member function pointer.
        template <class Target, class Func>
        struct member_function {
            member_function(Target target, Func func) :
                target(target),
                func(func)
            {
            }

            Target target;
            Func func;
        };

        // If the target object type supports it, define operator== for the
        // related member_function type(s). It is assumed that the function
        // objects are always comparable.
        template <class Target, class Func>
        inline typename boost::enable_if<ossie::internal::has_equals<Target>,bool>::type
        operator==(const member_function<Target,Func>& lhs, const member_function<Target,Func>& rhs)
        {
            return (lhs.target == rhs.target) && (lhs.func == rhs.func);
        }

        // The get_pointer() function converts the target of a member function
        // into a pointer so that any type can be used with operator->* to call
        // a member function pointer. This overlaps quite a bit with the Boost
        // function of the same name, but supports by-value objects in member
        // functions via its default implementation.
        template <class T>
        inline T* get_pointer(T& value)
        {
            return &value;
        }

        // Overload of get_pointer() for types that are already pointers
        template <class T>
        inline T* get_pointer(T* value)
        {
            return value;
        }

        // Overload of get_pointer() for boost::shared_ptr
        template <class T>
        inline T* get_pointer(boost::shared_ptr<T>& value)
        {
            return value.get();
        }

        // Templatized class that adapts a class instance/member function pair
        // to the function signature Sig. See function_invoker for explanation
        // about specialization requirements.
        template <class MemberFunc, class Sig>
        struct member_invoker;

        // Specialization for zero-argument invocation function
        template <class MemberFunc, class R>
        struct member_invoker<MemberFunc, R ()> {
            static R call(void* data)
            {
                MemberFunc* func = static_cast<MemberFunc*>(data);
                return (get_pointer(func->target)->*(func->func)) ();
            }
        };

        // Specialization for zero-argument void return invocation function
        template <class MemberFunc>
        struct member_invoker<MemberFunc, void ()> {
            static void call(void* data)
            {
                MemberFunc* func = static_cast<MemberFunc*>(data);
                (get_pointer(func->target)->*(func->func)) ();
            }
        };

        // Specialization for one-argument invocation function
        template <class MemberFunc, class R, class A1>
        struct member_invoker<MemberFunc, R (A1)> {
            static R call(void* data, A1 a1)
            {
                MemberFunc* func = static_cast<MemberFunc*>(data);
                return (get_pointer(func->target)->*(func->func)) (a1);
            }
        };

        // Specialization for one-argument void return invocation function
        template <class MemberFunc, class A1>
        struct member_invoker<MemberFunc, void (A1)> {
            static void call(void* data, A1 a1)
            {
                MemberFunc* func = static_cast<MemberFunc*>(data);
                (get_pointer(func->target)->*(func->func)) (a1);
            }
        };

        // Specialization for two-argument invocation function
        template <class MemberFunc, class R, class A1, class A2>
        struct member_invoker<MemberFunc, R (A1,A2)> {
            static R call(void* data, A1 a1, A2 a2)
            {
                MemberFunc* func = static_cast<MemberFunc*>(data);
                return (get_pointer(func->target)->*(func->func)) (a1, a2);
            }
        };

        // Specialization for one-argument void return invocation function
        template <class MemberFunc, class A1, class A2>
        struct member_invoker<MemberFunc, void (A1,A2)> {
            static void call(void* data, A1 a1, A2 a2)
            {
                MemberFunc* func = static_cast<MemberFunc*>(data);
                (get_pointer(func->target)->*(func->func)) (a1, a2);
            }
        };
    }

    /**
     * @brief Generic callback class.
     *
     * %callback provides overlapping functionality with Boost Function/Bind
     * and C++11's <functional> header (also available as part of TR1 on older
     * compilers); however, for the way callbacks are used in REDHAWK, each has
     * deficiencies that necessitated the creation of %callback:
     * - boost::function creates unique symbols per type that prevent the
     *   dynamic loader from unloading libraries
     * - C++11's std::function does not support operator==, which is used in
     *   the notification class to unregister a callback
     */
    template <class Sig>
    struct callback
    {
    private:
        typedef typename detail::callback_traits<Sig> traits;
        typedef typename traits::invoker_func invoker_func;
        typedef typename traits::result_type result_type;
        typedef typename traits::first_argument_type first_argument_type;
        typedef typename traits::second_argument_type second_argument_type;

        // Use a member pointer as the type for boolean-like conversion (so
        // that you can do "if (x)"), because it cannot be converted to any
        // other type
        typedef invoker_func (callback::*unspecified_bool_type);

    public:
        /**
         * @brief  Construct an empty %callback.
         */
        callback() :
            _M_invoker(0),
            _M_type(TYPE_NONE)
        {
        }

        /**
         * @brief  Construct a %callback with a function pointer.
         * @param func  Function pointer.
         *
         * The signature of @a func must be compatible with the declared return
         * type and arguments.
         */
        template <class Func>
        callback(Func* func) :
            _M_invoker(&detail::function_invoker<Func*,Sig>::call),
            _M_type(TYPE_FUNCTION)
        {
            // Function pointers are simple enough to store in _M_impl, but the
            // type doesn't match the function pointer placeholder; rather than
            // work around aliasing warnings, use placement new to initialize.
            typedef Func* impl_type;
            new (&_M_impl) impl_type(func);
        }

        /**
         * @brief  Construct a %callback with a functor by reference.
         * @param func  Reference to a functor object.
         *
         * The signature of @a func must be compatible with the declared return
         * type and arguments.
         */
        template <class Functor>
        callback(boost::reference_wrapper<Functor> func) :
            _M_invoker(&detail::function_invoker<Functor,Sig>::call),
            _M_type(TYPE_FUNCTOR_REF)
        {
            _M_impl.functor = func.get_pointer();
        }

        /**
         * @brief  Construct a %callback with a class instance and member
         *         function pointer.
         * @param target  Pointer to the class instance.
         * @param func  Member function pointer.
         *
         * The signature of @a func must be compatible with the declared return
         * type and arguments.
         */
        template <class Target, class Func>
        callback(Target* target, Func func) :
            _M_type(TYPE_MEMBER)
        {
            // Like function pointers, member functions with a pointer to an
            // object can be stored in _M_impl, but the types don't match.
            // Because the member_function template class is intentionally
            // laid out to match the member function placeholder, placement
            // new can be used to initialize.
            typedef detail::member_function<Target*,Func> impl_type;
            new (&_M_impl) impl_type(target, func);
            _M_invoker = &detail::member_invoker<impl_type,Sig>::call;
        }

        /**
         * @brief  Construct a %callback with a functor.
         * @param func  A functor object.
         *
         * @a func must be copy-constructible and support an operator() that is
         * compatible with the declared return type and arguments.
         */
        template <class Functor>
        callback(const Functor& func) :
            _M_invoker(&detail::function_invoker<Functor,Sig>::call),
            _M_type(TYPE_MANAGED)
        {
            // Create a new managed object, which also copies the functor
            // object argument
            _M_impl.managed = detail::void_manager::create(func);
        }

        /**
         * @brief  Construct a %callback with a class instance and member
         *         function pointer.
         * @param target  Class instance (or shared pointer to a class
         *                instance).
         * @param func  Member function pointer.
         *
         * The signature of @a func must be compatible with the declared return
         * type and arguments.
         */
        template <class Target, class Func>
        callback(Target target, Func func) :
            _M_type(TYPE_MANAGED)
        {
            // Create a new managed object, with a member_function as the
            // contents. This is most likely to be used with shared pointers,
            // but can work for other types as well
            typedef detail::member_function<Target,Func> impl_type;
            _M_impl.managed = detail::void_manager::create(impl_type(target, func));
            _M_invoker = &detail::member_invoker<impl_type,Sig>::call;
        }

        /**
         * Copy constructor.
         */
        callback(const callback& other) :
            _M_invoker(other._M_invoker),
            _M_type(other._M_type)
        {
            switch (other._M_type) {
            case TYPE_MANAGED:
                // Copy the other callback's implementation (which includes
                // the manager)
                _M_impl.managed = other._M_impl.managed->clone();
                break;
            case TYPE_FUNCTION:
                // Copy the function pointer (the type doesn't matter)
                _M_impl.func = other._M_impl.func;
                break;
            case TYPE_FUNCTOR_REF:
                // Copy the functor pointer (the type doesn't matter)
                _M_impl.functor = other._M_impl.functor;
                break;
            case TYPE_MEMBER:
                // Copy the member function object and pointer (the types don't
                // matter--the sizes are the same regardless of the object type
                // or function signature)
                _M_impl.member = other._M_impl.member;
                break;
            default:
                break;
            }
        }

        /**
         * @brief  Destructor.
         *
         * Any allocated objects are destroyed.
         */
        ~callback()
        {
            // Delete any managed object
            clear();
        }

        /**
         * Copy assignment.
         */
        callback& operator=(const callback& other)
        {
            if (&other != this) {
                // Use the copy constructor, swap and the destructor to handle
                // everything (as opposed to re-implementing basically the same
                // thing)
                callback temp(other);
                this->swap(temp);
            }
            return *this;
        }

        /**
         * @brief  Replace the current target with a class instance and member
         *         function pointer.
         * @param target  Class instance (by pointer, shared pointer or value).
         * @param func  Member function pointer.
         *
         * The signature of @a func must be compatible with the declared return
         * type and arguments.
         */
        template <class Target, class Func>
        void assign(Target target, Func func)
        {
            callback temp(target, func);
            this->swap(temp);
        }

        /**
         * @brief  Checks whether this %callback is equivalent to another.
         * @param other  Another %callback.
         * @return  true if this %callback is equivalent to @a other, false
         *          otherwise.
         *
         * Two callbacks are considered equal if their targets can reasonably
         * be assumed to be equivalent. The targets must be of the same type to
         * be compared:
         * - function pointers must be exactly equal
         * - member functions must point to the same object and member function
         * - functors must support operator== and evalute as equal
         */
        bool operator==(const callback& other) const
        {
            // If the invoker functions are different, the types must be
            // different
            if (_M_invoker != other._M_invoker) {
                return false;
            }
            switch (_M_type) {
            case TYPE_MANAGED:
                // Defer to the managed object's equality operator
                return (*(_M_impl.managed) == *(other._M_impl.managed));
            case TYPE_FUNCTION:
                // Compare standalone function pointers directly (the types
                // don't matter)
                return (_M_impl.func == other._M_impl.func);
            case TYPE_FUNCTOR_REF:
                // Compare standalone function pointers directly (the types
                // don't matter)
                return (_M_impl.functor == other._M_impl.functor);
            case TYPE_MEMBER:
                // Compare the object and member function pointers directly
                // (the types don't matter)
                return (_M_impl.member.target == other._M_impl.member.target) &&
                    (_M_impl.member.func == other._M_impl.member.func);
            default:
                // Empty callbacks are "equal"
                return true;
            }
        }

        /**
         * Returns true if this %callback is empty.
         */
        bool operator! () const
        {
            return empty();
        }

        /**
         * Evaluates to true in a boolean context if this %callback is non-
         * empty, false otherwise.
         */
        operator unspecified_bool_type () const
        {
            return empty()?0:&callback::_M_invoker;
        }

        /**
         * @brief  Returns true if this %callback does not have a target.
         */
        bool empty() const
        {
            return _M_type == TYPE_NONE;
        }

        /**
         * @brief  Resets this %callback to an empty state.
         */
        void clear()
        {
            if (_M_type == TYPE_MANAGED) {
                delete _M_impl.managed;
            }
            _M_invoker = 0;
            _M_type = TYPE_NONE;
        }

        /**
         * @brief  Swap contents with another %callback.
         * @param other  %callback to swap with.
         */
        void swap(callback& other)
        {
            // Copy the raw bytes, since we do not know (nor do we need to,
            // really) the other's implementation
            impl temp_impl;
            memcpy(&temp_impl, &other._M_impl, sizeof(impl));
            memcpy(&other._M_impl, &_M_impl, sizeof(impl));
            memcpy(&_M_impl, &temp_impl, sizeof(impl));

            std::swap(_M_invoker, other._M_invoker);
            std::swap(_M_type, other._M_type);
        }

        /**
         * @brief  Invokes the target function.
         * @return  The result of the target function (or nothing, if the
         *          return type of this %callback is void).
         * @throws  std::runtime_error if this %callback is empty
         */
        inline result_type operator() ()
        {
            if (empty()) {
                throw std::runtime_error("empty callback");
            }
            return _M_invoker(_M_data());
        }

        /**
         * @brief  Invokes the target function with a single argument.
         * @param a1  The first argument.
         * @return  The result of the target function (or nothing, if the
         *          return type of this %callback is void).
         * @throws  std::runtime_error if this %callback is empty
         */
        inline result_type operator() (first_argument_type a1)
        {
            if (empty()) {
                throw std::runtime_error("empty callback");
            }
            return _M_invoker(_M_data(), a1);
        }

        /**
         * @brief  Invokes the target function with a two arguments.
         * @param a1  The first argument.
         * @param a2  The second argument.
         * @return  The result of the target function (or nothing, if the
         *          return type of this %callback is void).
         * @throws  std::runtime_error if this %callback is empty
         */
        inline result_type operator() (first_argument_type a1, second_argument_type a2)
        {
            if (empty()) {
                throw std::runtime_error("empty callback");
            }
            return _M_invoker(_M_data(), a1, a2);
        }

    private:
        // Poison comparison to callbacks with different signatures
        template <class Sig2>
        bool operator==(const callback<Sig2>&) const;

        /// @cond IMPL

        // Returns a pointer to the callable object, for invocation
        void* _M_data()
        {
            if (_M_type == TYPE_MANAGED) {
                // The invoker takes the void_manager's contained object
                return _M_impl.managed->get_object();
            } else if (_M_type == TYPE_FUNCTOR_REF) {
                return _M_impl.functor;
            } else {
                // Use the impl object as an alias for the function pointer or
                // object/member pair
                return &_M_impl;
            }
        }

        // Discriminated union type to hold the specific implementation of the
        // callback. Function pointers, by-reference functors, and object
        // pointer/member function pointer pairs can be stored in-place; all
        // other types are allocated on the heap.
        typedef union {
            // Placeholder for a function pointer; all function pointers should
            // have the same size, so the specific type is only relevant for
            // dispatch, which does not go through this object
            void (*func)();

            // Pointer to a functor object; functors passed by reference (via
            // boost::ref) are converted to a pointer and stored here
            void* functor;

            // Placeholder for an object and member function pointer; as with
            // function pointers, the types are only relevant for dispatch,
            // which uses a different interpretation of this object
            struct {
                callback* target;
                void (callback::*func)();
            } member;

            // For all other types, which cannot be stored in place, a heap-
            // allocated void_manager holds the callable object
            detail::void_manager* managed;
        } impl;

        // Discriminant for implementation types
        enum impl_type {
            TYPE_NONE,
            TYPE_FUNCTION,
            TYPE_FUNCTOR_REF,
            TYPE_MEMBER,
            TYPE_MANAGED
        };

        impl _M_impl;
        invoker_func _M_invoker;
        impl_type _M_type;
        /// @endcond
    };

    template <class Sig, class T>
    bool operator!=(const callback<Sig>& lhs, const T& rhs)
    {
        return !(lhs == rhs);
    }
}

namespace ossie {

    // The functions and classes in this header are designed to provide easy
    // interfaces for binding a class instance and method for use as callbacks,
    // and 1-to-N notification functions (i.e., listeners). Argument and return
    // types are not required to match the declared function signature exactly,
    // as long as they can be converted automatically (e.g, int to float).
    //
    // The Boost function and bind libraries do most of the heavy lifting;
    // however, the compiler produces difficult-to-understand error messages
    // for instance methods where the bound function has an incompatible
    // signature with the expected function. Defining an invoker specifically
    // for the expected argument and return types produces a more legible error
    // message. Additionally, boost::bind requires the caller to explicitly
    // give the placeholder arguments.
    //
    // A design note: to implement remove for notifications, we need a way to
    // find the instance/method pair in the existing list of functions. Boost
    // functions do not support operator== against other Boost functions, but
    // they can be compared against some other callable, such as the result of
    // a call to boost::bind call. Unfortunately, in C++03, it is not possible
    // to declare the result of boost::bind as the return of another function;
    // as a result, we cannot implement a generalized function that returns a
    // callable object for use with both add and remove (this is why the one
    // and two argument notification class specializations explicitly make the
    // exact same boost::bind call).

    namespace detail {
        /*
         * Member function invoker class; forces the compiler to apply argument
         * type conversion so that incompatible method signatures yield useful
         * error messages.
         */
        template <class Signature>
        class invoker;

        /*
         * Template specialization for one-argument invoker.
         */
        template <class R, class A1>
        class invoker<R(A1)> {
        public:
            template <class Target, class Func>
            static R invoke(Target target, Func func, A1 arg1)
            {
                return (*target.*func)(arg1);
            }
        };

        /*
         * Template specialization for one-argument invoker returning void.
         */
        template <class A1>
        class invoker<void (A1)> {
        public:
            template <class Target, class Func>
            static void invoke(Target target, Func func, A1 arg1)
            {
                (*target.*func)(arg1);
            }
        };

        /*
         * Template specialization for two-argument invoker.
         */
        template <class R, class A1, class A2>
        class invoker<R(A1,A2)> {
        public:
            template <class Target, class Func>
            static R invoke(Target target, Func func, A1 arg1, A2 arg2)
            {
                return (*target.*func)(arg1, arg2);
            }
        };

        /*
         * Template specialization for two-argument invoker returning void.
         */
        template <class A1, class A2>
        class invoker<void (A1,A2)> {
        public:
            template <class Target, class Func>
            static void invoke(Target target, Func func, A1 arg1, A2 arg2)
            {
                (*target.*func)(arg1, arg2);
            }
        };
    };

    /*
     * Utility function to bind a member function and an instance of a class
     * together for use with a boost::function.
     *
     * Zero argument function version.
     */
    template <class R, class Target, class Func>
    void bind(boost::function<R()>& result, Target target, Func func)
    {
        result = boost::bind(func, target);
    }

    /*
     * Utility function to bind a member function and an instance of a class
     * together for use with a boost::function. Using this function, versus
     * using boost::bind directly, checks for compatibility with the function
     * signature before using boost::bind, which yields much better compiler
     * error messages when the functions are incompatible.
     *
     * One argument function version.
     */
    template <class R, class A1, class Target, class Func>
    void bind(boost::function<R(A1)>& result, Target target, Func func)
    {
        typedef ::ossie::detail::invoker<R(A1)> invoker_type;
        result = boost::bind(&invoker_type::template invoke<Target,Func>, target, func, _1);
    }

    /*
     * Utility function to bind a member function and an instance of a class
     * together for use with a boost::function. Using this function, versus
     * using boost::bind directly, checks for compatibility with the function
     * signature before using boost::bind, which yields much better compiler
     * error messages when the functions are incompatible.
     *
     * Two argument function version.
     */
    template <class R, class A1, class A2, class Target, class Func>
    void bind(boost::function<R(A1,A2)>& result, Target target, Func func)
    {
        typedef ::ossie::detail::invoker<R(A1,A2)> invoker_type;
        result = boost::bind(&invoker_type::template invoke<Target,Func>, target, func, _1, _2);
    }

    /*
     * The notification_base class contains the common members and methods to
     * implement arbitrary N-argument notifications.
     */
    template <class Signature>
    class notification_base
    {
    public:
        typedef redhawk::callback<Signature> func_type;

        /*
         * Register the callable 'func' to be called when this notification is
         * triggered.
         *
         * Listeners do not need to match the signature exactly, but the
         * notification's argument types must be convertible to the listener's
         * argument types. 
         */
        template <class Func>
        void add(Func func)
        {
            this->_M_listeners.push_back(func);
        }

        template <class Func>
        void remove(const Func& func)
        {
            this->_M_listeners.erase(std::remove(this->_M_listeners.begin(), this->_M_listeners.end(), func),
                                     this->_M_listeners.end());
        }

        /*
         * Register the member function 'func' to be called on the instance
         * 'target' when this notification is triggered.
         *
         * Listeners do not need to match the signature exactly, but the
         * notification's argument types must be convertible to the listener's
         * argument types. 
         */
        template <class Target, class Func>
        void add(Target target, Func func)
        {
            this->add(func_type(target, func));
        }

        /*
         * Remove the member function 'func' on the instance 'target' from
         * further notifications. The pair must have been registered together
         * in a prior call to add().
         */
        template <class Target, class Func>
        void remove(Target target, Func func)
        {
            this->remove(func_type(target, func));
        }

        bool empty() const
        {
            return _M_listeners.empty();
        }

    protected:
        std::list<func_type> _M_listeners;
    };

    /*
     * Generic notification class, templatized on the function signature.
     */
    template <class Signature>
    class notification;

    /*
     * Template specialization of zero-argument notifications.
     */
    template <class R>
    class notification<R()> : public notification_base<R()>
    {
    public:
        /*
         * Trigger the notification. All registered listeners will be called.
         */
        void operator() () {
            call_each(this->_M_listeners.begin(), this->_M_listeners.end());
        }

    private:
        template <class Iterator>
        static inline void call_each(Iterator begin, const Iterator end) {
            for (; begin != end; ++begin) {
                (*begin)();
            }
        }
    };

    /*
     * Template specialization of one-argument notifications.
     */
    template <class R, class A1>
    class notification<R(A1)> : public notification_base<R(A1)>
    {
    public:
        /*
         * Trigger the notification. All registered listeners will be called
         * with argument arg1.
         */
        void operator() (A1 arg1) {
            call_each(this->_M_listeners.begin(), this->_M_listeners.end(), arg1);
        }

    private:
        template <class Iterator>
        static inline void call_each(Iterator begin, const Iterator end, A1 arg1) {
            for (; begin != end; ++begin) {
                (*begin)(arg1);
            }
        }
    };

    /*
     * Template specialization of two-argument notifications.
     */
    template <class R, class A1, class A2>
    class notification<R(A1,A2)> : public notification_base<R(A1,A2)>
    {
    public:
        /*
         * Trigger the notification. All registered listeners will be called
         * with arguments arg1 and arg2.
         */
        void operator() (A1 arg1, A2 arg2) {
            call_each(this->_M_listeners.begin(), this->_M_listeners.end(), arg1, arg2);
        }

    private:
        template <class Iterator>
        static inline void call_each(Iterator begin, const Iterator end, A1 arg1, A2 arg2) {
            for (; begin != end; ++begin) {
                (*begin)(arg1, arg2);
            }
        }
    };
}

#endif // OSSIE_CALLBACK_H
