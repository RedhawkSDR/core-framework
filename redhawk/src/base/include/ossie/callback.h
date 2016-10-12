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
        typedef boost::function<Signature> func_type;

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
            super::add(boost::bind(func, target));
        }

        /*
         * Remove the member function 'func' on the instance 'target' from
         * further notifications. The pair must have been registered together
         * in a prior call to add().
         */
        template <class Target, class Func>
        void remove(Target target, Func func)
        {
            super::remove(boost::bind(func, target));
        }

    private:
        typedef notification_base<R()> super;

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
            super::add(boost::bind(&invoker_type::template invoke<Target,Func>, target, func, _1));
        }

        /*
         * Remove the member function 'func' on the instance 'target' from
         * further notifications. The pair must have been registered together
         * in a prior call to add().
         */
        template <class Target, class Func>
        void remove(Target target, Func func)
        {
            super::remove(boost::bind(&invoker_type::template invoke<Target,Func>, target, func, _1));
        }

    private:
        typedef notification_base<R(A1)> super;
        typedef ::ossie::detail::invoker<R(A1)> invoker_type;

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
            super::add(boost::bind(&invoker_type::template invoke<Target,Func>, target, func, _1, _2));
        }

        /*
         * Remove the member function 'func' on the instance 'target' from
         * further notifications. The pair must have been registered together
         * in a prior call to add().
         */
        template <class Target, class Func>
        void remove(Target target, Func func)
        {
            super::remove(boost::bind(&invoker_type::template invoke<Target,Func>, target, func, _1, _2));
        }

    private:
        typedef notification_base<R(A1,A2)> super;
        typedef ::ossie::detail::invoker<R(A1,A2)> invoker_type;

        template <class Iterator>
        static inline void call_each(Iterator begin, const Iterator end, A1 arg1, A2 arg2) {
            for (; begin != end; ++begin) {
                (*begin)(arg1, arg2);
            }
        }
    };
}

#endif // OSSIE_CALLBACK_H
