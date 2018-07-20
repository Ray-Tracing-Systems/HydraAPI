#pragma once

template<class T>
class aligned16
{
public:

  typedef size_t    size_type;
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
  typedef std::ptrdiff_t difference_type;
#elif defined _MSC_VER
  typedef ptrdiff_t difference_type;
#endif

  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;

  aligned16()                 = default;
  aligned16(const aligned16&) = default;

  pointer allocate(size_type n, const void* hint = nullptr)
  {
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
    return (pointer)::aligned_alloc(16, n*sizeof(T));
#elif defined _MSC_VER
    return (pointer)_aligned_malloc(n*sizeof(T), 16);
#endif
  }

  void deallocate(void* p, size_type n)
  {
    if (p != nullptr)
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
      free(p);
#elif defined _MSC_VER
      _aligned_free(p);
#endif
  }

  pointer           address(reference x) const { return &x; }
  const_pointer     address(const_reference x) const { return &x; }
  aligned16<T>&     operator=(const aligned16&) = default;

  void              construct(pointer p, const T& val)
  {
    new ((T*)p) T(val);
  }

  void              destroy(pointer p) { p->~T(); }
  size_type         max_size() const { return size_t(-1); }

  template <class U>
  struct rebind { typedef aligned16<U> other; };

  template <class U>
  explicit aligned16(const aligned16<U>&) {}

  template <class U>
  aligned16& operator=(const aligned16<U>&) { return *this; }

  bool operator==(const aligned16<T> a_rhs)       { return true; } // singleton, all instances are always equal
  bool operator==(const aligned16<T> a_rhs) const { return true; }
  bool operator!=(const aligned16& other)   const { return !(*this == other);}
};


