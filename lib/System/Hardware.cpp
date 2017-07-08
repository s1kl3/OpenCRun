
#include "opencrun/System/Hardware.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/ManagedStatic.h"

#include <unistd.h>

using namespace opencrun::sys;

//===----------------------------------------------------------------------===//
// HardwareComponent implementation.
//===----------------------------------------------------------------------===//

template <typename Ty, typename Ty_iterator, HardwareComponent::Type HWCompTy>
Ty_iterator HardwareComponent::begin() const {
  Hardware &HW = GetHardware();
  int nb_objs = hwloc_get_nbobjs_inside_cpuset_by_type(HW.GetHardwareTopology(),
      GetCPUSet(),
      hwloc_obj_type_t(HWCompTy));

  assert(nb_objs != -1 && "Multiple levels with the specified hardware object type!");
  if(!nb_objs)
    return Ty_iterator(this);

  // Get the first object of the type specified included in the same CPU set as the current
  // object.
  HardwareObject HWObj = hwloc_get_obj_inside_cpuset_by_type(
      HW.GetHardwareTopology(),
      GetCPUSet(),
      hwloc_obj_type_t(HWCompTy),
      0);
  HardwareComponent *HWComp = HW.GetHardwareComponent(HWObj);
  assert(HWComp && "Lacking hardware component!");

  return Ty_iterator(this, static_cast<Ty *>(HWComp));
}

template <typename Ty_iterator>
Ty_iterator HardwareComponent::end() const {
  return Ty_iterator(this);
}

template <unsigned CacheLevel, HardwareComponent::CacheType CacheTy>
HardwareComponent::const_cache_iterator HardwareComponent::cache_begin() const {
  Hardware &HW = GetHardware();
  int cache_depth = hwloc_get_cache_type_depth(HW.GetHardwareTopology(),
                                               CacheLevel,
                                               hwloc_obj_cache_type_t(CacheTy));
  assert((cache_depth != HWLOC_TYPE_DEPTH_MULTIPLE) &&
         "Multiple levels with the specified cache object type!");

  // No cache of the given level/type.
  if(cache_depth == HWLOC_TYPE_DEPTH_UNKNOWN)
    return const_cache_iterator(this,
                                CacheLevel,
                                CacheTy);

  HardwareObject HWObj = hwloc_get_obj_inside_cpuset_by_depth(HW.GetHardwareTopology(),
                                                              GetCPUSet(),
                                                              cache_depth,
                                                              0);
  HardwareComponent *HWComp = HW.GetHardwareComponent(HWObj);
  assert(HWComp && "Lacking hardware component!");

  return const_cache_iterator(this,
                              CacheLevel,
                              CacheTy,
                              static_cast<const HardwareCache *>(HWComp));
}

template <unsigned CacheLevel, HardwareComponent::CacheType CacheTy>
HardwareComponent::const_cache_iterator HardwareComponent::cache_end() const {
  return const_cache_iterator(this, CacheLevel, CacheTy);
}

HardwareComponent::HardwareComponentsContainer
HardwareComponent::GetChildren() const {
  Hardware &HW = GetHardware();
  HardwareComponentsContainer Children;

  for(unsigned I = 0; I < GetChildrenNum(); ++I) {
    // Get the corresponding HardwareComponent from Hardware's containers.
    HardwareComponent *HWComp = HW.GetHardwareComponent(HWObj->children[I]);
    assert(HWComp && "Lacking hardware component!");
    Children.push_back(HWComp);
  }

  return Children;
}

HardwareComponent *HardwareComponent::GetChild(unsigned I) const {
  Hardware &HW = GetHardware();

  assert(I < GetChildrenNum() && "Index out of range!");
  HardwareComponent *HWComp = HW.GetHardwareComponent(HWObj->children[I]);
  assert(HWComp && "Lacking hardware component!");
  return HWComp;
}

HardwareComponent *HardwareComponent::GetParent() const {
  Hardware &HW = GetHardware();
  HardwareObject ParentObj = HWObj->parent;

  // HWObj is the root object.
  if(!ParentObj)
    return NULL;

  HardwareComponent *Parent = HW.GetHardwareComponent(ParentObj);
  assert(Parent && "Lacking hardware component!");
  return Parent;
}

HardwareComponent *HardwareComponent::GetFirstChild() const {
  Hardware &HW = GetHardware();

  // HWObj has no children.
  if(!GetChildrenNum())
    return NULL;

  HardwareObject FstChldObj = HWObj->first_child;
  HardwareComponent *FstChld = HW.GetHardwareComponent(FstChldObj);
  assert(FstChld && "Lacking hardware component!");
  return FstChld;
}

HardwareComponent *HardwareComponent::GetLastChild() const {
  Hardware &HW = GetHardware();

  // HWObj has no children.
  if(!GetChildrenNum())
    return NULL;

  HardwareObject LstChldObj = HWObj->last_child;
  HardwareComponent *LstChld = HW.GetHardwareComponent(LstChldObj);
  assert(LstChld && "Lacking hardware component!");
  return LstChld;
}

HardwareComponent *HardwareComponent::GetNextSibling() const {
  Hardware &HW = GetHardware();

  // HWObj is the last sibling or it has no siblings.
  if(!HWObj->next_sibling)
    return NULL;

  HardwareObject NxtSblObj = HWObj->next_sibling;
  HardwareComponent *NxtSbl = HW.GetHardwareComponent(NxtSblObj);
  assert(NxtSblObj && "Lacking hardware component!");
  return NxtSbl;
}

HardwareComponent *HardwareComponent::GetPrevSibling() const {
  Hardware &HW = GetHardware();

  // HWObj is the first sibling or it has no siblings.
  if(!HWObj->prev_sibling)
    return NULL;

  HardwareObject PrvSblObj = HWObj->prev_sibling;
  HardwareComponent *PrvSbl = HW.GetHardwareComponent(PrvSblObj);
  assert(PrvSblObj && "Lacking hardware component!");
  return PrvSbl;
}

HardwareComponent *HardwareComponent::GetNextCousin() const {
  Hardware &HW = GetHardware();
  
  // HWObj is the last cousin or it has no cousins.
  if(!HWObj->next_cousin)
    return NULL;

  HardwareObject NxtCousObj = HWObj->next_cousin;
  HardwareComponent *NxtCous = HW.GetHardwareComponent(NxtCousObj);
  assert(NxtCousObj && "Lacking hardware component!");
  return NxtCous;
}

HardwareComponent *HardwareComponent::GetPrevCousin() const {
  Hardware &HW = GetHardware();

  // HWObj is the first cousin or it has no cousins.
  if(!HWObj->prev_cousin)
    return NULL;

  HardwareObject PrvCousObj = HWObj->prev_cousin;
  HardwareComponent *PrvCous = HW.GetHardwareComponent(PrvCousObj);
  assert(PrvCousObj && "Lacking hardware component!");
  return PrvCous;
}

//===----------------------------------------------------------------------===//
// HardwareMachine implementation.
//===----------------------------------------------------------------------===//

HardwareMachine::const_node_iterator HardwareMachine::node_begin() const {
  return begin<const HardwareNode, const_node_iterator, HardwareComponent::Node>();
}

HardwareMachine::const_node_iterator HardwareMachine::node_end() const {
  return end<const_node_iterator>();
}

HardwareMachine::const_socket_iterator HardwareMachine::socket_begin() const {
  return begin<const HardwareSocket, const_socket_iterator, HardwareComponent::Socket>();
}

HardwareMachine::const_socket_iterator HardwareMachine::socket_end() const {
  return end<const_socket_iterator>();
}

HardwareMachine::const_cpu_iterator HardwareMachine::cpu_begin() const {
  return begin<const HardwareCPU, const_cpu_iterator, HardwareComponent::CPU>();
}

HardwareMachine::const_cpu_iterator HardwareMachine::cpu_end() const {
  return end<const_cpu_iterator>();
}
const HardwareNode &HardwareMachine::node_front() const {
  return *node_begin();
}

const HardwareNode &HardwareMachine::node_back() const {
  const_node_iterator I = node_begin(),
                      E = node_end(),
                      J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

const HardwareSocket &HardwareMachine::socket_front() const {
  return *socket_begin();
}

const HardwareSocket &HardwareMachine::socket_back() const {
  const_socket_iterator I = socket_begin(),
                        E = socket_end(),
                        J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

const HardwareCPU &HardwareMachine::cpu_front() const {
  return *cpu_begin();
}

const HardwareCPU &HardwareMachine::cpu_back() const {
  const_cpu_iterator I = cpu_begin(),
                     E = cpu_end(),
                     J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

//===----------------------------------------------------------------------===//
// HardwareNode implementation.
//===----------------------------------------------------------------------===//

HardwareNode::const_socket_iterator HardwareNode::socket_begin() const {
  return begin<const HardwareSocket, const_socket_iterator, HardwareComponent::Socket>();
}

HardwareNode::const_socket_iterator HardwareNode::socket_end() const {
  return end<const_socket_iterator>();
}

const HardwareSocket &HardwareNode::socket_front() const {
  return *socket_begin();
}

const HardwareSocket &HardwareNode::socket_back() const {
  const_socket_iterator I = socket_begin(),
                        E = socket_end(),
                        J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

//===----------------------------------------------------------------------===//
// HardwareSocket implementation.
//===----------------------------------------------------------------------===//

const HardwareCache *HardwareSocket::GetLastCache() const {
  const HardwareComponent *HWComp = this;
  const HardwareCache *HWCache = NULL;
  
  for(; HWComp; HWComp = HWComp->GetFirstChild())
    if(HWComp->GetType() == HardwareComponent::Cache) {
      HWCache = static_cast<const HardwareCache *>(HWComp);
      break;
    }

  return HWCache;
}

HardwareSocket::const_cache_iterator HardwareSocket::llc_begin() const {
  const HardwareCache *HWCache = GetLastCache();
  assert(HWCache && "No hardware cache present!");

  return const_cache_iterator(this,
      HWCache->GetLevel(),
      HWCache->GetCacheType(),
      HWCache);
}

HardwareSocket::const_cache_iterator HardwareSocket::llc_end() const {
  const HardwareCache *HWCache = GetLastCache();

  return const_cache_iterator(this,
                              HWCache->GetLevel(),
                              HWCache->GetCacheType());
}

HardwareSocket::const_cache_iterator HardwareSocket::l1ic_begin() const {
  return cache_begin<1, HardwareComponent::Instruction>();
}

HardwareSocket::const_cache_iterator HardwareSocket::l1ic_end() const {
  return cache_end<1, HardwareComponent::Instruction>();
}

HardwareSocket::const_cache_iterator HardwareSocket::l1dc_begin() const {
  return cache_begin<1, HardwareComponent::Data>();
}

HardwareSocket::const_cache_iterator HardwareSocket::l1dc_end() const {
  return cache_end<1, HardwareComponent::Data>();
}

HardwareSocket::const_cpu_iterator HardwareSocket::cpu_begin() const {
  return begin<const HardwareCPU, const_cpu_iterator, HardwareComponent::CPU>();
}

HardwareSocket::const_cpu_iterator HardwareSocket::cpu_end() const {
  return end<const_cpu_iterator>();
}

const HardwareCache &HardwareSocket::llc_front() const {
  return *llc_begin();
}

const HardwareCache &HardwareSocket::llc_back() const {
  const_cache_iterator I = llc_begin(),
                       E = llc_end(),
                       J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

const HardwareCache &HardwareSocket::l1ic_front() const {
  return *l1ic_begin();
}

const HardwareCache &HardwareSocket::l1ic_back() const {
  const_cache_iterator I = l1ic_begin(),
                       E = l1ic_end(),
                       J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

const HardwareCache &HardwareSocket::l1dc_front() const {
  return *l1dc_begin();
}

const HardwareCache &HardwareSocket::l1dc_back() const {
  const_cache_iterator I = l1dc_begin(),
                       E = l1dc_end(),
                       J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

unsigned HardwareSocket::GetLastCacheLevel() const {
  const HardwareCache *HWCache = GetLastCache();
  assert(HWCache && "No hardware cache present!");
  return HWCache->GetLevel();
}

HardwareCache::CacheType HardwareSocket::GetLastCacheType() const {
  const HardwareCache *HWCache = GetLastCache();
  assert(HWCache && "No hardware cache present!");
  return HWCache->GetCacheType();
}

//===----------------------------------------------------------------------===//
// HardwareCache implementation.
//===----------------------------------------------------------------------===//

HardwareCache::const_smtcpu_iterator HardwareCache::smtcpu_begin() const {
  return begin<const HardwareSMTCPU, const_smtcpu_iterator, HardwareComponent::SMTCPU>();
}

HardwareCache::const_smtcpu_iterator HardwareCache::smtcpu_end() const {
  return end<const_smtcpu_iterator>();
}

const HardwareSMTCPU &HardwareCache::smtcpu_front() const {
  return *smtcpu_begin();
}

const HardwareSMTCPU &HardwareCache::smtcpu_back() const {
  const_smtcpu_iterator I = smtcpu_begin(),
                        E = smtcpu_end(),
                        J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

//===----------------------------------------------------------------------===//
// HardwareSMTCPU implementation.
//===----------------------------------------------------------------------===//

HardwareSMTCPU::const_cpu_iterator HardwareSMTCPU::cpu_begin() const {
  return begin<const HardwareCPU, const_cpu_iterator, HardwareComponent::CPU>();
}

HardwareSMTCPU::const_cpu_iterator HardwareSMTCPU::cpu_end() const {
  return end<const_cpu_iterator>();
}

const HardwareCPU &HardwareSMTCPU::cpu_front() const {
  return *cpu_begin();
}

const HardwareCPU &HardwareSMTCPU::cpu_back() const {
  const_cpu_iterator I = cpu_begin(),
                     E = cpu_end(),
                     J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

//===----------------------------------------------------------------------===//
// HardwareCPU implementation.
//===----------------------------------------------------------------------===//

HardwareCache *HardwareCPU::GetFirstLevelCache() const {
  return GetCache(1);
}

HardwareCache *HardwareCPU::GetCache(unsigned Level) const {
  HardwareComponent *Comp = GetParent();

  // Comp is NULL when the root of the hardware topology is reached.
  while(Comp) {
    // Check if the current component is a cache memory of the given
    // level.
    if(HardwareCache *Cache = llvm::dyn_cast<HardwareCache>(Comp)) {
      if(Cache->GetLevel() == Level) 
        return Cache;
    }

    Comp = Comp->GetParent();
  }

  return NULL;
}

HardwareNode *HardwareCPU::GetNUMANode() const {
  HardwareComponent *Comp = GetParent();

  // Comp is NULL when the root of the hardware topology is reached.
  while(Comp) {
    // Check if the current component is a NUMA node.
    if(HardwareNode *Node = llvm::dyn_cast<HardwareNode>(Comp))
      return Node;

    Comp = Comp->GetParent();
  }

  return NULL;
}

//===----------------------------------------------------------------------===//
// Hardware implementation.
//===----------------------------------------------------------------------===//

size_t Hardware::GetPageSize() {
  return getpagesize();
}

size_t Hardware::GetCacheLineSize() {
  Hardware &HW = GetHardware();
  Hardware::cache_iterator I = HW.cache_begin(),
                           E = HW.cache_end();

  return I != E ? I->GetLineSize() : 0;
}

size_t Hardware::GetMaxNaturalAlignment() {
  size_t Alignment;

  #if defined(__x86_64__) || defined(__i386__)
  Alignment = 16;
  #else
  #error "architecture not supported"
  #endif

  return Alignment;
}

Hardware::Hardware() {
  // Topology allocation and initialization.
  hwloc_topology_init(&HWTop);

  // Topology detection.
  hwloc_topology_load(HWTop);

  // Walk the topology and build an HardwareComponent for
  // each node.
  unsigned max_depth = hwloc_topology_get_depth(HWTop);
  for(unsigned depth = 0; depth < max_depth; depth++) {
    for (unsigned i = 0; i < hwloc_get_nbobjs_by_depth(HWTop, depth); i++) {
      HardwareComponent::HardwareObject HWObj = hwloc_get_obj_by_depth(HWTop, depth, i);
      HardwareComponent *HWComp =
        BuildHardwareComponent(HWObj);
      
      // TODO: Handle NULL HWComp (unknown hardware component).
      
      HWMap[HWObj] = HWComp;
      HWComps.insert(HWComp);
    }
  }
}

Hardware::~Hardware() {
  // Delete all created HardwareComponent objects. 
  llvm::DeleteContainerPointers(HWComps);

  // Topology destruction.
  hwloc_topology_destroy(HWTop);
}

HardwareComponent *Hardware::GetRootHardwareComponent() {
  HardwareComponent::HardwareObject HWObj = hwloc_get_root_obj(HWTop);

  return GetHardwareComponent(HWObj);
}

HardwareComponent *Hardware::GetHardwareComponent(HardwareComponent::HardwareObject HWObj) {
  if(HWMap.count(HWObj))
    return HWMap[HWObj];

  return NULL;
}

HardwareComponent *Hardware::BuildHardwareComponent(HardwareComponent::HardwareObject HWObj) {
  switch(HWObj->type) {
    case HardwareComponent::System:
      return new HardwareSystem(HWObj);
    case HardwareComponent::Machine:
      return new HardwareMachine(HWObj);
    case HardwareComponent::Node:
      return new HardwareNode(HWObj);
    case HardwareComponent::Socket:
      return new HardwareSocket(HWObj);
    case HardwareComponent::Cache:
      return new HardwareCache(HWObj);
    case HardwareComponent::SMTCPU:
      return new HardwareSMTCPU(HWObj);
    case HardwareComponent::CPU:
      return new HardwareCPU(HWObj);
    case HardwareComponent::Group:
      return new HardwareGroup(HWObj);
    case HardwareComponent::Misc:
      return new HardwareMisc(HWObj);
    case HardwareComponent::Bridge:
      return new HardwareBridge(HWObj);
    case HardwareComponent::PCIDevice:
      return new HardwarePCIDevice(HWObj);
    case HardwareComponent::OSDevice:
      return new HardwareOSDevice(HWObj);

    // Wrong component type.
    case HardwareComponent::TypeMax:
    default:
      return NULL;
  }
}

llvm::ManagedStatic<Hardware> HW;

Hardware &opencrun::sys::GetHardware() {
  return *HW;
}
