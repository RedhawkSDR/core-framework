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

#include "CallbackTest.h"

#include <complex>
#include <cmath>

#include <boost/make_shared.hpp>

#include <ossie/callback.h>

CPPUNIT_TEST_SUITE_REGISTRATION(CallbackTest);

namespace {
    // Simple global function and counter to test zero-argument functions
    static int global_counter = 0;
    static int increment()
    {
        return ++global_counter;
    }

    // Convert a C string to uppercase in-place, returning the number of
    // characters that were modified
    static int upcase(char* str)
    {
        int count = 0;
        for (char* pos = str; *pos != '\0'; ++pos) {
            char ch = *pos;
            if (!isupper(ch)) {
                ++count;
                *pos = toupper(ch);
            }
        }
        return count;
    }

    // Simple functor that returns a constant value
    template <typename T>
    struct Constant {
        Constant(T value) :
            _value(value)
        {
        }

        T operator() () const
        {
            return _value;
        }

        void set(T value)
        {
            _value = value;
        }

        bool operator==(const Constant& other) const
        {
            return (_value == other._value);
        }

    private:
        T _value;
    };

    // Functor that multiplies argument by a pre-defined scale value
    template <class T>
    class Scale {
    public:
        Scale(T scale) :
            _scale(scale)
        {
        }

        T operator() (T arg)
        {
            return arg * _scale;
        }
    private:
        T _scale;
    };

    // Functor that returns the average of two integers
    struct Average {
        float operator() (int arg1, int arg2)
        {
            return (arg1 + arg2) / 2.0;
        }
    };

    // Object that provides some const and non-const member functions to test
    // various modes of member function callbacks
    class Object {
    public:
        Object(const std::string& name) :
            _name(name),
            _total(0)
        {
        }

        std::string name() const
        {
            return _name;
        }

        int total() const
        {
            return _total;
        }

        int reset()
        {
            int result = _total;
            _total = 0;
            return result;
        }

        int add(int count)
        {
            _total += count;
            return _total;
        }

        int subtract(int count)
        {
            _total -= count;
            return _total;
        }

        int clamp(int min, int max)
        {
            _total = std::max(min, std::min(max, _total));
            return _total;
        }

    private:
        const std::string _name;
        int _total;
    };

    // Class to help test by-reference arguments and returns
    template <class R, class A1, class A2>
    class ArgumentTester {
    public:
        ArgumentTester(R result) :
            _result(result),
            _arg1(0),
            _arg2(0)
        {
        }

        R& call(A1& arg1, A2& arg2)
        {
            _arg1 = &arg1;
            _arg2 = &arg2;
            return _result;
        }

        R* result()
        {
            return &_result;
        }

        A1* arg1()
        {
            return _arg1;
        }

        A2* arg2()
        {
            return _arg2;
        }

    private:
        R _result;
        A1* _arg1;
        A2* _arg2;
    };

    // Functor that takes two arguments by reference and modified them:
    // - an integer, whose sign gets inverted
    // - a string, that gets reversed
    struct Mutator {
        void operator() (int& number, std::string& message)
        {
            number *= -1;
            std::string out;
            std::copy(message.rbegin(), message.rend(), std::back_inserter(out));
            message = out;
        }
    };
}

void CallbackTest::setUp()
{
}

void CallbackTest::tearDown()
{
}

void CallbackTest::testEmpty()
{
    // Default constructor creates an empty callback
    redhawk::callback<void ()> func;
    CPPUNIT_ASSERT(func.empty());

    // Assign a value; should not be empty
    func = &getpid;
    CPPUNIT_ASSERT(!func.empty());

    // Clear should reset to empty
    func.clear();
    CPPUNIT_ASSERT(func.empty());
}

void CallbackTest::testEmptyCall()
{
    // Calling an empty callback should always throw a runtime error

    redhawk::callback<int ()> func0;
    CPPUNIT_ASSERT(func0.empty());
    CPPUNIT_ASSERT_THROW(func0(), std::runtime_error);

    redhawk::callback<std::string (const std::string&)> func1;
    CPPUNIT_ASSERT(func1.empty());
    CPPUNIT_ASSERT_THROW(func1("abc"), std::runtime_error);

    redhawk::callback<double (double,int)> func2;
    CPPUNIT_ASSERT(func2.empty());
    CPPUNIT_ASSERT_THROW(func2(0.0, 0), std::runtime_error);
}

void CallbackTest::testBooleanOperators()
{
    // Empty callback, should evaluate to boolean false
    redhawk::callback<int ()> func;
    CPPUNIT_ASSERT(func.empty());
    CPPUNIT_ASSERT(!func);
    if (func) {
        // Boolean-like conversion has to be tested in the context of an if
        // statement to ensure we are testing what we expect (CPPUNIT_ASSERT
        // expands to something like "if (!cond) {...}", which we're already
        // testing above)
        CPPUNIT_FAIL("Boolean-like conversion returned true on empty callback");
    }

    // Empty callback, should evaluate to boolean true
    func = &getpid;
    CPPUNIT_ASSERT(!func.empty());
    CPPUNIT_ASSERT_EQUAL(false, !func);
    if (func) {
        // Expected behavior
    } else {
        CPPUNIT_FAIL("Boolean-like conversion returned false on valid callback");
    }
}

void CallbackTest::testFunction()
{
    // Zero-argument function: getpid()
    redhawk::callback<pid_t ()> func0(&getpid);
    pid_t self = func0();
    CPPUNIT_ASSERT_EQUAL(getpid(), self);

    // One-argument function: complex conjugate
    typedef std::complex<float> complex_float;
    redhawk::callback<complex_float (complex_float)> func1 = &std::conj<float>;
    complex_float conj_result = func1(complex_float(1.0, 1.0));
    CPPUNIT_ASSERT_EQUAL(complex_float(1.0, -1.0), conj_result);

    // Two-argument function: power (with integer exponent)
    redhawk::callback<double (double, int)> func2 = &std::pow<double,int>;
    double pow_result = func2(7.0, 3);
    CPPUNIT_ASSERT_EQUAL(343.0, pow_result);
}

void CallbackTest::testFunctionEquals()
{
    // Function pointer callbacks should only be equal if the function pointers
    // are exactly the same
    redhawk::callback<int ()> func0 = &getuid;
    CPPUNIT_ASSERT(func0 != &getpid);
    CPPUNIT_ASSERT(func0 == &getuid);
}

void CallbackTest::testFunctor()
{
    // Zero argument functor: return a pre-defined value
    redhawk::callback<std::string ()> func0 = Constant<std::string>("test");
    std::string message = func0();
    CPPUNIT_ASSERT_EQUAL(std::string("test"), message);

    // One-argument functor: scale argument by a numeric factor
    redhawk::callback<int (int)> func1 = Scale<int>(1);
    int result = func1(2);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One argument functor returned incorrect result", 2, result);

    // Two-argument functor: average of two numbers
    redhawk::callback<float (int,int)> func2 = Average();
    float avg = func2(7, 10);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Two argument functor returned incorrect result", 8.5f, avg);
}

void CallbackTest::testFunctorRef()
{
    // Associate a functor by reference; changes to the functor should be
    // reflected in subsequent calls
    Constant<short> constant(-5);
    redhawk::callback<short ()> func = boost::ref(constant);
    CPPUNIT_ASSERT(func == boost::ref(constant));
    short value = func();
    CPPUNIT_ASSERT_EQUAL((short) -5, value);
    constant.set(10);
    value = func();
    CPPUNIT_ASSERT_EQUAL((short) 10, value);
}

void CallbackTest::testFunctorEquals()
{
    // Constant has an operator== defined, that compares the constant value
    redhawk::callback<std::string ()> func0_a = Constant<std::string>("same");
    redhawk::callback<std::string ()> func0_b = Constant<std::string>("same");
    CPPUNIT_ASSERT(func0_a == func0_b);
    CPPUNIT_ASSERT(func0_a != Constant<std::string>("other"));

    // Scale has no operator== defined, so compare always fails
    redhawk::callback<double (double)> func1_a = Scale<double>(2.0);
    redhawk::callback<double (double)> func1_b = Scale<double>(2.0);
    CPPUNIT_ASSERT(func1_a != func1_b);

    // Average has no operator==, but references compare based on whether the
    // references are to the same object
    Average avg;
    redhawk::callback<float (int,int)> func2_a = boost::ref(avg);
    redhawk::callback<float (int,int)> func2_b = boost::ref(avg);
    CPPUNIT_ASSERT(func2_a == func2_b);
    Average avg2;
    CPPUNIT_ASSERT(func2_a != boost::ref(avg2));
}

void CallbackTest::testMemberFunction()
{
    boost::shared_ptr<Object> obj = boost::make_shared<Object>("test");

    // Zero-argument callback; use a const pointer alias, and a const-qualified
    // member function
    const Object* obj_ptr = obj.get();
    redhawk::callback<std::string ()> func0(obj_ptr, &Object::name);
    CPPUNIT_ASSERT_EQUAL(obj->name(), func0());

    // One-argument member function; check that the object is modified as
    // expected
    redhawk::callback<int (int)> func1(obj, &Object::add);
    int result = func1(10);
    CPPUNIT_ASSERT_EQUAL(10, result);
    func1(20);
    CPPUNIT_ASSERT_EQUAL(30, obj->total());

    // Use assign to rebind the callback (using a non-const pointer instead of
    // the shared pointer)
    func1.assign(obj.get(), &Object::subtract);
    func1(15);
    CPPUNIT_ASSERT_EQUAL(15, obj->total());

    // Use an object by value (implicitly making a copy); the function should
    // work as expected, without affecting the original object (yes, this is a
    // contrived example)
    redhawk::callback<int (int,int)> func2(*obj, &Object::clamp);
    result = func2(0, 10);
    CPPUNIT_ASSERT_EQUAL(10, result);
    CPPUNIT_ASSERT_EQUAL(15, obj->total());
}

void CallbackTest::testMemberFunctionEquals()
{
    // Member functions should only be equal if both the target object and
    // function are the same
    Object obj1("first");
    redhawk::callback<int (int)> func1(&obj1, &Object::add);
    redhawk::callback<int (int)> func2(&obj1, &Object::add);
    CPPUNIT_ASSERT(func1 == func2);

    // Same object, different function should be unequal
    func2.assign(&obj1, &Object::subtract);
    CPPUNIT_ASSERT_MESSAGE("Different member functions should not be equal", func1 != func2);

    // Same function, different object should be unequal
    Object obj2("second");
    func2.assign(&obj2, &Object::add);
    CPPUNIT_ASSERT_MESSAGE("Different target objects should not be equal", func1 != func2);
}

void CallbackTest::testMixedEquals()
{
    redhawk::callback<int (int)> function = &std::abs<float>;

    Object obj("member");
    redhawk::callback<int (int)> member(&obj, &Object::add);

    Scale<int> scale(1);
    redhawk::callback<int (int)> functor = scale;
    redhawk::callback<int (int)> functor_ref = boost::ref(scale);

    // Test all possible combinations
    CPPUNIT_ASSERT(function != member);
    CPPUNIT_ASSERT(function != functor);
    CPPUNIT_ASSERT(function != functor_ref);
    CPPUNIT_ASSERT(member != functor);
    CPPUNIT_ASSERT(member != functor_ref);
    CPPUNIT_ASSERT(functor != functor_ref);
}

void CallbackTest::testReferenceArguments()
{
    // Use a functor that modifies the passed-in arguments to check that the
    // original arguments are modifed (as opposed to some argument value on the
    // stack)
    redhawk::callback<void (int&, std::string&)> functor = Mutator();
    int value = 1;
    std::string name = "test text";
    functor(value, name);
    CPPUNIT_ASSERT_EQUAL(-1, value);
    CPPUNIT_ASSERT_EQUAL(std::string("txet tset"), name);

    // Use a member function with const arguments (by reference) to makes sure
    // that they are passed unmodified (i.e., no copies)
    typedef std::string R;
    typedef const int A1;
    typedef const Object A2;
    typedef ArgumentTester<R, A1, A2> TesterType;

    TesterType tester("references");
    redhawk::callback<R& (A1&, A2&)> member(&tester, &TesterType::call);

    int number = 0;
    Object obj("argument");
    R& result = member(number, obj);
    CPPUNIT_ASSERT(&number == tester.arg1());
    CPPUNIT_ASSERT(&obj == tester.arg2());
    CPPUNIT_ASSERT(&result == tester.result());
}

void CallbackTest::testArgumentConversion()
{
    // One-argument member function: abuse constant value functor and its set
    // function to implicitly construct a C++ string from a C string
    Constant<std::string> message("Test");
    redhawk::callback<void (const char*)> func1(&message, &Constant<std::string>::set);
    func1("Updated");
    CPPUNIT_ASSERT_EQUAL(std::string("Updated"), message());

    // Two-argument functor: implicit conversion of arguments from double to
    // int (truncation to -1, 6), plus implict conversion of result from float
    // to int (2.5 truncates to 2)
    redhawk::callback<int (double,double)> func2 = Average();
    int avg = func2(-1.25, 6.375);
    CPPUNIT_ASSERT_EQUAL(2, avg);
}

void CallbackTest::testVoidReturn()
{
    // Test to ensure that functions which return a value can be assigned to
    // callbacks that return void. From a C++ implementation standpoint, there
    // are template specializations to ensure that callback invoker functions
    // returning void are syntactically correct--a void function can return the
    // result of calling another void function, but if that function has a
    // return value it is a syntax error.

    // Zero-argument function: ignore the result of increment(), defined above,
    // but check that it takes effect
    global_counter = 0;
    redhawk::callback<void ()> func0 = &increment;
    func0();
    CPPUNIT_ASSERT_EQUAL(1, global_counter);

    // Zero-argument member function: ignore the result of Object::reset() but
    // check it it takes effect
    Object obj("void");
    obj.add(5);
    func0.assign(&obj, &Object::reset);
    func0();
    CPPUNIT_ASSERT_EQUAL(0, obj.total());

    // One-argument function: ignore the result of upcase(), defined above
    redhawk::callback<void (char*)> func1 = &upcase;
    char buf[64];
    sprintf(buf, "message");
    func1(buf);
    CPPUNIT_ASSERT(strcmp(buf, "MESSAGE") == 0);

    // One-argument member function
    redhawk::callback<void (int)> func1_member(&obj, &Object::add);
    func1_member(1.25);
    CPPUNIT_ASSERT_EQUAL(1, obj.total());

    // Two-argument function: ignore the result of strcpy (which is just a char
    // * to the destination, anyway), but check that it worked
    redhawk::callback<void (char*, const char*)> func2 = &strcpy;
    const char* expected = "expected value";
    func2(buf, expected);
    CPPUNIT_ASSERT(strcmp(buf, expected) == 0);

    // Two-argument member function
    obj.add(100);
    redhawk::callback<void (int, int)> func2_member(&obj, &Object::clamp);
    func2_member(0, 50);
    CPPUNIT_ASSERT_EQUAL(50, obj.total());
}
