#ifndef ATOMIC_COUNTER_H
#define ATOMIC_COUNTER_H

template <typename T>
class atomic_counter {
public:
    typedef T counter_type;

    atomic_counter() :
        _value(0)
    {
    }

    explicit atomic_counter(counter_type value) :
        _value(value)
    {
    }

    operator counter_type() const
    {
#ifdef __ATOMIC_RELAXED
        counter_type result;
        __atomic_load(&_value, &result, __ATOMIC_RELAXED);
        return result;
#else
        return _value;
#endif
    }

    atomic_counter& operator=(counter_type value)
    {
#ifdef __ATOMIC_RELAXED
        __atomic_store(&_value, &value, __ATOMIC_RELAXED);
#else
        _value = value;
#endif
        return *this;
    }

    counter_type increment()
    {
#ifdef __ATOMIC_RELAXED
        return __atomic_add_fetch(&_value, 1, __ATOMIC_RELAXED);
#else
        return __sync_add_and_fetch(&_value, 1);
#endif
    }

    counter_type decrement()
    {
#ifdef __ATOMIC_RELAXED
        return __atomic_sub_fetch(&_value, 1, __ATOMIC_RELAXED);
#else
        return __sync_sub_and_fetch(&_value, 1);
#endif        
    }

private:
    volatile counter_type _value;
};

#endif // ATOMIC_COUNTER_H
