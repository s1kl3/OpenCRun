#ifndef OPENCRUN_UTIL_FOOTPRINT_H
#define OPENCRUN_UTIL_FOOTPRINT_H

namespace opencrun {

class Footprint {
public:
  Footprint() : LocalMemoryUsage(0),
                PrivateMemoryUsage(0),
                TempMemoryUsage(0) { }

public:
  size_t GetMaxWorkGroupSize(size_t AvailablePrivateMemory) const {
    size_t MaxWorkItems;

    // Estimate by giving to each work-item the same amount of private memory.
    if(PrivateMemoryUsage)
      MaxWorkItems = AvailablePrivateMemory / PrivateMemoryUsage;

    // We do not have precise information about the amount of private memory
    // used by the work-items, since we need to compute an upper bound, suppose
    // each work-item uses at least 1 byte.
    else
      MaxWorkItems = AvailablePrivateMemory;

    return MaxWorkItems;
  }

  size_t GetLocalMemoryUsage() const {
    return LocalMemoryUsage;
  }

  size_t GetPrivateMemoryUsage() const {
    return PrivateMemoryUsage;
  }

  float GetPrivateMemoryUsageAccuracy() const {
    size_t TotalMemory = PrivateMemoryUsage + TempMemoryUsage;
    float Accuracy;

    // Estimate: fraction of private over all data (private + temporaries).
    if(TotalMemory != 0)
      Accuracy = static_cast<float>(PrivateMemoryUsage) / TotalMemory;

    // Perfect estimate, maybe useless.
    else
      Accuracy = 1.0;

    return Accuracy;
  }

  void AddLocalMemoryUsage(size_t Usage) {
    LocalMemoryUsage += Usage;
  }

protected:
  void AddPrivateMemoryUsage(size_t Usage) {
    PrivateMemoryUsage += Usage;
  }

  void AddTempMemoryUsage(size_t Usage) {
    TempMemoryUsage += Usage;
  }

private:
  size_t LocalMemoryUsage;
  size_t PrivateMemoryUsage;
  size_t TempMemoryUsage;
};

}

#endif
