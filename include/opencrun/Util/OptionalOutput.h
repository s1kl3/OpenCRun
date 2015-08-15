#ifndef OPTIONAL_OUTPUT_H
#define OPTIONAL_OUTPUT_H

#include <utility>

namespace opencrun {

template<typename Ty>
class OptionalOutput {
public:
  explicit OptionalOutput(Ty &Val, Ty *Ptr = nullptr)
   : Val(Val), Ptr(Ptr) {}

  ~OptionalOutput() {
    if (Ptr)
      *Ptr = Val;
  }

  const Ty &get() const { return Val; }
  void set(const Ty &x) { Val = x; }

private:
  Ty &Val;
  Ty *Ptr;
};

}

#endif
