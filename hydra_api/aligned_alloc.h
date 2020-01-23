#pragma once

#include <cstddef>

#ifndef WIN32
  #include <stdlib.h>
#endif // WIN32

namespace cvex
{

template <class T, int N>
class aligned
{

public:

  typedef T value_type;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  pointer allocate(size_type n, const void* hint = nullptr)
  {
    #ifdef WIN32
    return (pointer)_aligned_malloc(n*sizeof(T), N);
    #else
    return (pointer)aligned_alloc(N, n*sizeof(T));
    #endif
  }

  void deallocate(void* p, size_type n)
  {
    if (p != nullptr)
    {
    #ifdef WIN32
      _aligned_free(p);
    #else
      free(p);
    #endif // WIN32
    }
  }

  template <class U> struct rebind { typedef aligned<U,N> other; };
  inline aligned() throw() {}
  inline aligned(const aligned&) throw() {}
  template <class U> inline aligned(const aligned<U,N>&) throw() {}
  inline ~aligned() throw() {}

  inline pointer address(reference r) { return &r; }
  inline const_pointer address(const_reference r) const { return &r; }
  inline void construct(pointer p, const_reference value) { new (p) value_type(value); }
  inline void destroy(pointer p) { p->~value_type(); }
  inline size_type max_size() const throw() { return size_type(-1) / sizeof(T); }
  inline bool operator==(const aligned&) { return true; }
  inline bool operator!=(const aligned& rhs) { return !operator==(rhs); }
};

}