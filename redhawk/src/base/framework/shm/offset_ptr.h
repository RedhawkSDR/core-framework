#ifndef OFFSET_PTR_HH
#define OFFSET_PTR_HH

#include <cstddef>

template <typename T>
inline T* offset_ptr(void* base, ptrdiff_t offset)
{
    char* ptr = reinterpret_cast<char*>(base);
    ptr += offset;
    return reinterpret_cast<T*>(ptr);
}

template <typename T>
inline const T* offset_ptr(const void* base, ptrdiff_t offset)
{
    const char* ptr = reinterpret_cast<const char*>(base);
    ptr += offset;
    return reinterpret_cast<const T*>(ptr);
}

#endif // OFFSET_PTR_HH
