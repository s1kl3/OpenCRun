#ifndef OPENCRUN_UTIL_SETUNIQUEPTR_H
#define OPENCRUN_UTIL_SETUNIQUEPTR_H

#include <memory>

namespace opencrun {

template<typename T, typename D = std::default_delete<T>>
class set_unique_ptr {
  intptr_t Value;
  D deleter;

  static_assert(std::alignment_of<T>::value > 1,
                "not enough low bits for tagging deleter semantic.");

  static intptr_t compose(T *p, bool no_delete) {
    const uintptr_t mask = 1;
    return (intptr_t(p) & ~mask) | (no_delete & mask);
  }

public:
  using pointer = T*;
  using element_type = T;
  using deleter_type = D;

  explicit set_unique_ptr(pointer p = nullptr,
                          bool no_delete = false) noexcept
   : Value(compose(p, no_delete)) {}

  set_unique_ptr(set_unique_ptr &&u) noexcept
   : Value(compose(u.release(), u.is_nodelete())),
     deleter(std::forward<deleter_type>(u.get_deleter())) {}

  template<typename U, typename E>
  set_unique_ptr(std::unique_ptr<U, E> &&ptr) noexcept
   : Value(compose(ptr.release(), false)),
     deleter(std::forward<E>(ptr.get_deleter())) {}

  ~set_unique_ptr() {
    if (!is_nodelete() && get())
      deleter(get());
    Value = 0;
  }

  const deleter_type &get_deleter() const noexcept {
    return deleter;
  }
  deleter_type &get_deleter() noexcept {
    return deleter;
  }

  pointer get() const {
    return reinterpret_cast<T*>(Value & ~uintptr_t(1));
  }

  bool is_nodelete() const {
    return Value & uintptr_t(1);
  }

  pointer release() {
    auto p = get();
    Value = compose(nullptr, is_nodelete());
    return p;
  }

  operator bool() const noexcept {
    return get() != nullptr;
  }

  operator pointer() const {
    return get();
  }

  element_type &operator*() const {
    assert(get() != nullptr);
    return *get();
  }

  pointer operator->() const {
    assert(get() != nullptr);
    return get();
  }

  void reset(pointer p = nullptr) {
    auto nodelete = is_nodelete();
    if (!nodelete && get())
      deleter(get());
    Value = compose(p, nodelete);
  }
};

template<typename T, typename D, typename U, typename E>
inline bool operator==(const set_unique_ptr<T, D> &x,
                       const set_unique_ptr<U, E> &y) {
  return x.get() == y.get();
}

template<typename T, typename D>
inline bool operator==(const set_unique_ptr<T, D> &x, std::nullptr_t) noexcept {
  return !x;
}

template<typename T, typename D>
inline bool operator==(std::nullptr_t, const set_unique_ptr<T, D> &x) noexcept {
  return !x;
}

template<typename T, typename D, typename U, typename E>
inline bool operator!=(const set_unique_ptr<T, D> &x,
                       const set_unique_ptr<U, E> &y) {
  return x.get() != y.get();
}

template<typename T, typename D>
inline bool operator!=(const set_unique_ptr<T, D> &x, std::nullptr_t) noexcept {
  return (bool)x;
}

template<typename T, typename D>
inline bool operator!=(std::nullptr_t, const set_unique_ptr<T, D> &x) noexcept {
  return (bool)x;
}

template<typename T, typename D, typename U, typename E>
inline bool operator<(const set_unique_ptr<T, D> &x,
                      const set_unique_ptr<U, E> &y) {
  using _CT =
    typename std::common_type<typename set_unique_ptr<T, D>::pointer,
                              typename set_unique_ptr<U, E>::pointer>::type;
  return std::less<_CT>()(x.get(), y.get());
}

template<typename T, typename D>
inline bool operator<(const set_unique_ptr<T, D> &x, std::nullptr_t) {
  return std::less<typename set_unique_ptr<T, D>::pointer>()(x.get(), nullptr);
}

template<typename T, typename D>
inline bool operator<(std::nullptr_t, const set_unique_ptr<T, D> &x) {
  return std::less<typename set_unique_ptr<T, D>::pointer>()(nullptr, x.get());
}

template<typename T, typename D, typename U, typename E>
inline bool operator<=(const set_unique_ptr<T, D> &x,
                       const set_unique_ptr<U, E> &y) {
  return !(y < x);
}

template<typename T, typename D>
inline bool operator<=(const set_unique_ptr<T, D> &x, std::nullptr_t) {
  return !(nullptr < x);
}

template<typename T, typename D>
inline bool operator<=(std::nullptr_t, const set_unique_ptr<T, D> &x) {
  return !(x < nullptr);
}

template<typename T, typename D, typename U, typename E>
inline bool operator>(const set_unique_ptr<T, D> &x,
                      const set_unique_ptr<U, E> &y) {
  return (y < x);
}

template<typename T, typename D>
inline bool operator>(const set_unique_ptr<T, D> &x, std::nullptr_t) {
  return (nullptr < x);
}

template<typename T, typename D>
inline bool operator>(std::nullptr_t, const set_unique_ptr<T, D> &x) {
  return (x < nullptr);
}

template<typename T, typename D, typename U, typename E>
inline bool operator>=(const set_unique_ptr<T, D> &x,
                      const set_unique_ptr<U, E> &y) {
  return !(x < y);
}

template<typename T, typename D>
inline bool operator>=(const set_unique_ptr<T, D> &x, std::nullptr_t) {
  return !(x < nullptr);
}

template<typename T, typename D>
inline bool operator>=(std::nullptr_t, const set_unique_ptr<T, D> &x) {
  return !(nullptr < x);
}

template<typename T>
set_unique_ptr<T> make_find_ptr(T *p) {
  return set_unique_ptr<T>{p, true};
}

}

#endif
