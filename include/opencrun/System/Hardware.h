
#ifndef OPENCRUN_SYSTEM_HARDWARE_H
#define OPENCRUN_SYSTEM_HARDWARE_H

#include "llvm/Support/Casting.h"
#include "llvm/ADT/StringRef.h"

#include <vector>
#include <map>
#include <set>

#include <hwloc.h>

namespace opencrun {
namespace sys {

//===----------------------------------------------------------------------===//
// Hardware object adaptors hierarchy.
//===----------------------------------------------------------------------===//

class HardwareNode;
class HardwareSocket;
class HardwareCache;
class HardwareSMTCPU;
class HardwareCPU;

class HardwareComponent {
public:
  enum Type {
    System = HWLOC_OBJ_SYSTEM,
    Machine = HWLOC_OBJ_MACHINE,
    Node = HWLOC_OBJ_NODE,

    Socket = HWLOC_OBJ_SOCKET,

    Cache = HWLOC_OBJ_CACHE,

    SMTCPU = HWLOC_OBJ_CORE,
    CPU = HWLOC_OBJ_PU,

    Group = HWLOC_OBJ_GROUP,
    Misc = HWLOC_OBJ_MISC,

    Bridge = HWLOC_OBJ_BRIDGE,
    PCIDevice = HWLOC_OBJ_PCI_DEVICE,
    OSDevice = HWLOC_OBJ_OS_DEVICE,

    TypeMax = HWLOC_OBJ_TYPE_MAX
  };

  enum CacheType {
    Unified = HWLOC_OBJ_CACHE_UNIFIED,
    Data = HWLOC_OBJ_CACHE_DATA,
    Instruction = HWLOC_OBJ_CACHE_INSTRUCTION
  };

public:
  typedef hwloc_obj_t HardwareObject;
  typedef hwloc_cpuset_t CPUSet;
  typedef hwloc_nodeset_t NodeSet;
  typedef std::vector<HardwareComponent *> HardwareComponentsContainer;

public:
  template <typename Ty>
  class ConstFilteredIterator {
  public:
    ConstFilteredIterator(const HardwareComponent *HWCompRoot, Ty *HWComp = NULL) : HWCompRoot(HWCompRoot),
                                                                                    HWComp(HWComp) { }

    ~ConstFilteredIterator() { }

  public:
    bool operator==(const ConstFilteredIterator &That) const {
      return (HWCompRoot == That.HWCompRoot) &&
             (HWComp == That.HWComp);
    }

    bool operator!=(const ConstFilteredIterator &That) const {
      return !(*this == That);
    }

    const Ty &operator*() const {
      return *HWComp;    
    }

    const Ty *operator->() const {
      return HWComp; 
    }

    // Pre-increment.
    ConstFilteredIterator<Ty> &operator++() {
      Advance(); return *this;
    }

    // Post-increment.
    ConstFilteredIterator<Ty> operator++(int Ign) {
      ConstFilteredIterator<Ty> That = *this; ++*this; return That;
    }

  private:
    void Advance();

  private:
    const HardwareComponent *HWCompRoot;
    Ty *HWComp;
  };

  class ConstCacheIterator : public ConstFilteredIterator<const HardwareCache> {
  public:
    ConstCacheIterator(const HardwareComponent *HWCompRoot,
                       unsigned CacheLevel,
                       CacheType CacheTy,
                       const HardwareCache *HWCache = NULL) :
      ConstFilteredIterator<const HardwareCache>(HWCompRoot, HWCache),
      CacheLevel(CacheLevel),
      CacheTy(CacheTy) { }

  public:
    bool operator==(const ConstCacheIterator &That) {
      return ConstFilteredIterator<const HardwareCache>::operator==(That) &&
             (CacheLevel == That.CacheLevel) &&
             (CacheTy == That.CacheTy);
    }
    
  private:
    // Cache level and type to seek for.
    unsigned CacheLevel;
    CacheType CacheTy;
  };

public:
  typedef ConstFilteredIterator<const HardwareNode> const_node_iterator;
  typedef ConstFilteredIterator<const HardwareSocket> const_socket_iterator;
  typedef ConstFilteredIterator<const HardwareSMTCPU> const_smtcpu_iterator;
  typedef ConstFilteredIterator<const HardwareCPU> const_cpu_iterator;

  typedef ConstCacheIterator const_cache_iterator;

public:
  template <typename Ty, typename Ty_iterator, Type HWCompTy> Ty_iterator begin() const;
  template <typename Ty_iterator> Ty_iterator end() const;

  template <unsigned CacheLevel, CacheType CacheTy> const_cache_iterator cache_begin() const;
  template <unsigned CacheLevel, CacheType CacheTy> const_cache_iterator cache_end() const;

protected:
  HardwareComponent(HardwareObject HWObj, Type Ty) : HWObj(HWObj),
                                                     ComponentTy(Ty) { }

public:
  Type GetType() const { return ComponentTy; }

  HardwareObject GetHardwareObject() const { return HWObj; }

  llvm::StringRef GetName() const { return llvm::StringRef(HWObj->name); }

  unsigned GetNumCoveredCPUs() const { return hwloc_bitmap_weight(HWObj->cpuset); }

  unsigned GetDepth() const { return HWObj->depth; }
  unsigned GetOSIndex() const { return HWObj->os_index; }
  unsigned GetLogicalIndex() const { return HWObj->logical_index; }
  unsigned GetSiblingIndex() const { return HWObj->sibling_rank; }
  unsigned GetChildrenNum() const { return HWObj->arity; }

  CPUSet GetCPUSet() const { return HWObj->cpuset; }
  NodeSet GetNodeSet() const { return HWObj->nodeset; }

  unsigned long GetTotalMemorySize() const { return HWObj->memory.total_memory; }
  unsigned long GetLocalMemorySize() const { return HWObj->memory.local_memory; }

  HardwareComponentsContainer GetChildren() const;
  HardwareComponent *GetChild(unsigned I) const; 

  HardwareComponent *GetParent() const;
  HardwareComponent *GetFirstChild() const;
  HardwareComponent *GetLastChild() const;
  HardwareComponent *GetNextSibling() const;
  HardwareComponent *GetPrevSibling() const;
  HardwareComponent *GetNextCousin() const;
  HardwareComponent *GetPrevCousin() const;

protected:
  HardwareObject HWObj;

private:
  Type ComponentTy;
};

//
// System - The whole system that is accessible to hwloc and that may be a cluster
// of machines.
//

class HardwareSystem : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::System;
  }

public:
  HardwareSystem(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::System) { }
};

//
// Machine - The typical root object of the hardware topology tree. A set of processors
// and memory with cache coherency.
//

class HardwareMachine : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::Machine;
  }

public:
  typedef HardwareComponent::const_node_iterator const_node_iterator;
  typedef HardwareComponent::const_socket_iterator const_socket_iterator;
  typedef HardwareComponent::const_cpu_iterator const_cpu_iterator;

public:

  const_node_iterator node_begin() const;
  const_node_iterator node_end() const;

  const_socket_iterator socket_begin() const;
  const_socket_iterator socket_end() const;

  const_cpu_iterator cpu_begin() const;
  const_cpu_iterator cpu_end() const;

  const HardwareNode &node_front() const;
  const HardwareNode &node_back() const;

  const HardwareSocket &socket_front() const;
  const HardwareSocket &socket_back() const;

  const HardwareCPU &cpu_front() const;
  const HardwareCPU &cpu_back() const;

public:
  HardwareMachine(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::Machine) { }
};

//
// NUMA node - A set of processors around memory which the processor can directly
// access.
//

class HardwareNode : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::Node;
  }

public:
  typedef HardwareComponent::const_socket_iterator const_socket_iterator;

public:
  const_socket_iterator socket_begin() const;
  const_socket_iterator socket_end() const;

  const HardwareSocket &socket_front() const;
  const HardwareSocket &socket_back() const;

public:
  HardwareNode(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::Node) { }

public:
  unsigned GetNodeID() const { return GetLogicalIndex(); }
};

//
// Socket - The physical package or chip that can be removed physically.
//

class HardwareSocket : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::Socket;
  }

public:
  typedef HardwareComponent::const_cache_iterator const_cache_iterator;
  typedef HardwareComponent::const_cpu_iterator const_cpu_iterator;

public:
  const_cache_iterator llc_begin() const;
  const_cache_iterator llc_end() const;

  const_cache_iterator l1ic_begin() const;
  const_cache_iterator l1ic_end() const;

  const_cache_iterator l1dc_begin() const;
  const_cache_iterator l1dc_end() const;

  const_cpu_iterator cpu_begin() const;
  const_cpu_iterator cpu_end() const;

public:
  const HardwareCache &llc_front() const;
  const HardwareCache &llc_back() const;

  const HardwareCache &l1ic_front() const;
  const HardwareCache &l1ic_back() const;

  const HardwareCache &l1dc_front() const;
  const HardwareCache &l1dc_back() const;

public:
  HardwareSocket(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::Socket) { }

public:
  const HardwareCache *GetLastCache() const;
  unsigned GetLastCacheLevel() const;
  CacheType GetLastCacheType() const; 
};

//
// Cache - Can be cache L1i, L1d, L2, L3, ...
//

class HardwareCache : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::Cache;
  }

public:
  typedef HardwareComponent::const_smtcpu_iterator const_smtcpu_iterator;

public:
  const_smtcpu_iterator smtcpu_begin() const;
  const_smtcpu_iterator smtcpu_end() const;

  const HardwareSMTCPU &smtcpu_front() const;
  const HardwareSMTCPU &smtcpu_back() const;

public:
  HardwareCache(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::Cache) { }

public:
  unsigned GetLevel() const { return HWObj->attr->cache.depth; }
  bool IsFirstLevel() const { return (HWObj->attr->cache.depth) == 1; }
  size_t GetSize() const { return HWObj->attr->cache.size; }
  size_t GetLineSize() const { return HWObj->attr->cache.linesize; }
  bool IsFullyAssociative() const { return (HWObj->attr->cache.associativity) == -1; }
  CacheType GetCacheType() const { return CacheType(HWObj->attr->cache.type); }
};

//
// SMTCPU - A physical core that, in case of SMT, is shared by several logical processors. In the
// hwloc terminology is referred to as a core.
//

class HardwareSMTCPU : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::SMTCPU;
  }

public:
  typedef HardwareComponent::const_cpu_iterator const_cpu_iterator;

public:
  const_cpu_iterator cpu_begin() const;
  const_cpu_iterator cpu_end() const;

  const HardwareCPU &cpu_front() const;
  const HardwareCPU &cpu_back() const;

public:
  HardwareSMTCPU(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::SMTCPU) { }
};

//
// CPU - A Processing Unit (PU) or logical core. It may share a physical core with other
// logical cores).
//

class HardwareCPU : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::CPU;
  }

public:
  HardwareCPU(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::CPU) { }

public:
  unsigned GetCoreID() const { return GetLogicalIndex(); }
  HardwareCache *GetFirstLevelCache() const;
  HardwareCache *GetCache(unsigned Level) const;
  HardwareNode *GetNUMANode() const;
};

//
// Others - Hardware components that may be used in the future.
//

class HardwareGroup : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::Group;
  }

public:
  HardwareGroup(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::Group) { }
};

class HardwareMisc : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::Misc;
  }

public:
  HardwareMisc(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::Misc) { }
};

class HardwareBridge : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::Bridge;
  }

public:
  HardwareBridge(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::Bridge) { }
};

class HardwarePCIDevice : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::PCIDevice;
  }

public:
  HardwarePCIDevice(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::PCIDevice) { }
};

class HardwareOSDevice : public HardwareComponent {
public:
  static bool classof(const HardwareComponent *Comp) {
    return Comp->GetType() == HardwareComponent::OSDevice;
  }

public:
  HardwareOSDevice(HardwareObject HWObj)
    : HardwareComponent(HWObj, HardwareComponent::OSDevice) { }
};

//===----------------------------------------------------------------------===//
// Hardware topology adaptor.
//===----------------------------------------------------------------------===//

class Hardware {
public:
  static size_t GetPageSize();
  static size_t GetCacheLineSize();
  static size_t GetMaxNaturalAlignment();

public:
  typedef hwloc_topology_t HardwareTopology;
  typedef std::map<HardwareComponent::HardwareObject, HardwareComponent *> HardwareObjToCompMap;
  typedef std::set<HardwareComponent *> HardwareComponentsContainer;

  typedef HardwareComponentsContainer::iterator component_iterator;
  typedef HardwareComponentsContainer::const_iterator const_component_iterator;

public:
  component_iterator component_begin() { return HWComps.begin(); }
  component_iterator component_end() { return HWComps.end(); }

  const_component_iterator component_begin() const { return HWComps.begin(); }
  const_component_iterator component_end() const { return HWComps.end(); }

public:
  Hardware();
  ~Hardware();

public:
  template <typename Ty>
  class FilteredComponentIterator {
  public:
    FilteredComponentIterator(component_iterator I,
                              component_iterator E) : I(I),
                                                      E(E) {
      // Find first valid position.
      if(I != E && !llvm::isa<Ty>(*this->I))
        Advance();
    }

  public:
    bool operator==(const FilteredComponentIterator<Ty> &That) const {
      return I == That.I && E == That.E;
    }
    bool operator!=(const FilteredComponentIterator<Ty> &That) const {
      return !(*this == That);
    }

    Ty &operator*() const { return static_cast<Ty &>(*I.operator*()); }
    Ty *operator->() const { return static_cast<Ty *>(*I.operator->()); }

    // Pre-increment.
    FilteredComponentIterator<Ty> &operator++() {
      Advance(); return *this;
    }

    // Post-increment.
    FilteredComponentIterator<Ty> operator++(int Ign) {
      FilteredComponentIterator<Ty> That = *this; ++*this; return That;
    }

  private:
    void Advance() {
      if(I == E)
        return;

      do { ++I; } while(I != E && !llvm::isa<Ty>(*I));
    }

  private:
    component_iterator I;
    component_iterator E;
  };

public:
  typedef FilteredComponentIterator<HardwareMachine> machine_iterator;
  typedef FilteredComponentIterator<HardwareNode> node_iterator;
  typedef FilteredComponentIterator<HardwareNode> socket_iterator;
  typedef FilteredComponentIterator<HardwareCache> cache_iterator;
  typedef FilteredComponentIterator<HardwareSMTCPU> smtcpu_iterator;
  typedef FilteredComponentIterator<HardwareCPU> cpu_iterator;

  // More iterators could be added for the remaining specific hardware
  // components.

public:
  machine_iterator machine_begin() {
    return machine_iterator(component_begin(), component_end());
  }
  machine_iterator machine_end() {
    return machine_iterator(component_end(), component_end());
  }

  node_iterator node_begin() {
    return node_iterator(component_begin(), component_end());
  }
  node_iterator node_end() {
    return node_iterator(component_end(), component_end());
  }

  socket_iterator socket_begin() {
    return socket_iterator(component_begin(), component_end());
  }
  socket_iterator socket_end() {
    return socket_iterator(component_end(), component_end());
  }

  cache_iterator cache_begin() {
    return cache_iterator(component_begin(), component_end());
  }
  cache_iterator cache_end() {
    return cache_iterator(component_end(), component_end());
  }

  smtcpu_iterator smtcpu_begin() {
    return smtcpu_iterator(component_begin(), component_end());
  }
  smtcpu_iterator smtcpu_end() {
    return smtcpu_iterator(component_end(), component_end());
  }

  cpu_iterator cpu_begin() {
    return cpu_iterator(component_begin(), component_end());
  }
  cpu_iterator cpu_end() {
    return cpu_iterator(component_end(), component_end());
  }

public:
  HardwareTopology &GetHardwareTopology() { return HWTop; }

public:
  HardwareComponent *GetRootHardwareComponent();
  HardwareComponent *GetHardwareComponent(HardwareComponent::HardwareObject HWObj);

private:
  HardwareComponent *BuildHardwareComponent(HardwareComponent::HardwareObject HWObj);

private:
  HardwareTopology HWTop;
  HardwareObjToCompMap HWMap;
  HardwareComponentsContainer HWComps;
};

Hardware &GetHardware();

} // End namespace sys.
} // End namespace opencrun.

#endif // OPENCRUN_SYSTEM_HARDWARE_H
