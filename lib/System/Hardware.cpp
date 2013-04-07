
#include "opencrun/System/Hardware.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"

#include <fstream>
#include <string>
#include <unistd.h>

using namespace opencrun::sys;

namespace {

llvm::error_code getFileSafe(llvm::StringRef Filename, std::string &result) {
  std::ifstream fs(Filename.str().c_str(), std::fstream::in);
  if (!fs.is_open()) return llvm::make_error_code(llvm::errc::io_error);


  std::string cnt((std::istreambuf_iterator<char>(fs)), 
                      std::istreambuf_iterator<char>());

  fs.close();

  result.swap(cnt);

  return llvm::error_code::success();
}

class LinuxHardwareParser {
public:
  typedef llvm::SmallPtrSet<HardwareCache *, 4> CacheContainer;
  typedef llvm::DenseMap<unsigned, CacheContainer> CacheByLevelIndex;

public:
  LinuxHardwareParser(Hardware::CPUComponents &CPUs,
                      Hardware::CacheComponents &Caches,
                      Hardware::NodeComponents &Nodes) : CPUs(CPUs),
                                                         Caches(Caches),
                                                         Nodes(Nodes) { }

  void operator()() {

    llvm::Twine CPUsRoot = "/sys/devices/system/cpu";
    llvm::error_code ErrCode;

    llvm::sys::fs::directory_iterator I(CPUsRoot, ErrCode),
                                      E;
    do {
      if(ErrCode)
        llvm::report_fatal_error("Cannot read CPUs info");

      ParseCPU(I->path());

      I.increment(ErrCode);
    } while(I != E);
  }

private:
  HardwareCPU *ParseCPU(llvm::StringRef Path) {
    llvm::FoldingSetNodeID ID;
    void *InsertPoint;
    llvm::error_code ErrCode;

    // Must start with "cpu".
    llvm::StringRef Name = llvm::sys::path::filename(Path);
    if(!Name.startswith("cpu"))
      return NULL;

    // Must end with the CPU identifier.
    unsigned CoreID;
    if(Name.substr(3).getAsInteger(0, CoreID))
      return NULL;

    // Try looking if CPU has already been added.
    ID.AddInteger(CoreID);
    HardwareCPU *CPU = CPUs.FindNodeOrInsertPos(ID, InsertPoint);

    // CPU not yet built.
    if(!CPU) {
      CPU = new HardwareCPU(CoreID);
      CPUs.InsertNode(CPU, InsertPoint);
    }

    llvm::SmallString<32> CachesRoot;

    llvm::sys::path::append(CachesRoot, Path, "cache");
    llvm::sys::fs::directory_iterator I(CachesRoot.str(), ErrCode),
                                      E;

    // Look at cache hierarchy.
    CacheByLevelIndex CacheLevelIndex;
    do {
      if(ErrCode)
        llvm::report_fatal_error("Cannot read CPU info");

      if(HardwareCache *Cache = ParseCache(I->path())) {
        unsigned Level = Cache->GetLevel();

        CacheLevelIndex[Level].insert(Cache);

        // First level cache directly connected to the CPU.
        if(Cache->IsFirstLevelCache()) {
          CPU->AddLink(Cache);
          Cache->AddLink(CPU);

        // Link all previous level caches to this cache.
        } else if(CacheLevelIndex.count(Level - 1)) {
          CacheContainer &PrevLevel = CacheLevelIndex[Level - 1];
          for(CacheContainer::iterator I = PrevLevel.begin(),
                                       E = PrevLevel.end();
                                       I != E;
                                       ++I) {
            (*I)->AddLink(Cache);
            Cache->AddLink(*I);
          }
        }
      }

      I.increment(ErrCode);
    } while(I != E);

    llvm::SmallString<32> NodeRoot;
    bool SysFsInfoAvailable;

    HardwareNode *Node;

    llvm::sys::path::append(NodeRoot, Path, "node");

    // The sys fs does not export info about NUMA nodes: fall-back to proc fs.
    if(llvm::sys::fs::is_directory(NodeRoot.str(), SysFsInfoAvailable) ||
       !SysFsInfoAvailable)
       Node = ParseProcFsNode("/proc");

    // We can gather NUMA information from the sys fs.
    else
      Node = ParseSysFsNode(NodeRoot);

    unsigned LastLevel = CacheLevelIndex.size();

    if(!CacheLevelIndex.count(LastLevel))
      llvm::report_fatal_error("Cache-less architectures not yet supported");

    // Each CPU has only a last level cache.
    HardwareCache *LLC = *CacheLevelIndex[LastLevel].begin();

    // Link node with the LLC.
    LLC->AddLink(Node);
    Node->AddLink(LLC);

    return CPU;
  }

  HardwareCache *ParseCache(llvm::StringRef Path) {
    llvm::FoldingSetNodeID ID;
    void *InsertPoint;

    // Must start with "index".
    llvm::StringRef Name = llvm::sys::path::filename(Path);
    if(!Name.startswith("index"))
      return NULL;

    // Must end with the cache index for the current architecture.
    unsigned Index;
    if(Name.substr(5).getAsInteger(0, Index))
      return NULL;

    unsigned Level;
    HardwareCache::Kind Kind;
    HardwareComponent::LinksContainer SharedCPUs;

    // Get info about the cache.
    ParseCacheLevel(Path, Level);
    ParseCacheKind(Path, Kind);
    ParseCacheSharedCPUs(Path, SharedCPUs);

    // Check if the cache has already been built.
    ID.AddInteger(Level);
    ID.AddInteger(Kind);
    for(HardwareComponent::LinksContainer::iterator I = SharedCPUs.begin(),
                                                    E = SharedCPUs.end();
                                                    I != E;
                                                    ++I)
      ID.AddPointer(*I);
    HardwareCache *Cache = Caches.FindNodeOrInsertPos(ID, InsertPoint);

    // Cache not yet built.
    if(!Cache) {
      Cache = new HardwareCache(Level, Kind, SharedCPUs);
      Caches.InsertNode(Cache, InsertPoint);

      size_t Size, LineSize;

      ParseCacheSize(Path, Size);
      ParseCacheLineSize(Path, LineSize);

      Cache->SetSize(Size);
      Cache->SetLineSize(LineSize);
    }

    return Cache;
  }

  HardwareNode *ParseProcFsNode(llvm::StringRef Path) {
    llvm::FoldingSetNodeID ID;
    void *InsertPoint;

    llvm::SmallString<32> MemInfoPath;
    llvm::sys::path::append(MemInfoPath, Path, "meminfo");

    std::string Buf;

    if(getFileSafe(MemInfoPath, Buf))
      llvm::report_fatal_error("Cannot read meminfo file");

    // Check if node has already been built.
    ID.AddInteger(0);
    HardwareNode *Node = Nodes.FindNodeOrInsertPos(ID, InsertPoint);

    // Node not yet built.
    if(!Node) {
      Node = new HardwareNode(0);
      Nodes.InsertNode(Node, InsertPoint);

      llvm::StringRef Input(Buf);

      llvm::SmallVector<llvm::StringRef, 64> Lines;
      Input.split(Lines, "\n");

      for(llvm::SmallVector<llvm::StringRef, 64>::iterator I = Lines.begin(),
                                                           E = Lines.end();
                                                           I != E;
                                                           ++I) {

        if(I->startswith("MemTotal")) {
          unsigned long long Size;
          const char *K = NULL, *J, *T;

          T = I->end();

          // Look for first digit.
          for(J = I->begin(); J != T && !K; ++J)
            if(std::isdigit(*J))
              K = J;

          if(!K)
            llvm::report_fatal_error("Corrupted meminfo file");

          // Look for the last digit.
          for(J = K; J != T && std::isdigit(*J); ++J) { }
          llvm::StringRef RawSize(K, J - K);

          // Look for the multiplier.
          if(++J >= T)
            llvm::report_fatal_error("Corrupted meminfo file");

          llvm::StringRef Multiplier(J, T - J);

          // Cannot fail, already parsed.
          RawSize.getAsInteger(0, Size);

          // Add multiplier.
          Size *= llvm::StringSwitch<unsigned long long>(Multiplier)
                    .Case("kB", 1024)
                    .Default(0);

          if(!Size)
            llvm::report_fatal_error("Cannot parse memory size");

          Node->SetMemorySize(Size);
        }
      }
    }

    return Node;
  }

  HardwareNode *ParseSysFsNode(llvm::StringRef Path) {
    llvm_unreachable("Not yet implemented");
  }

  void ParseCacheLevel(llvm::StringRef CachePath, unsigned &Level) {
    llvm::SmallString<32> LevelPath;
    llvm::sys::path::append(LevelPath, CachePath, "level");

    std::string Buf;

    if(getFileSafe(LevelPath, Buf))
      llvm::report_fatal_error("Cannot read cache level file");

    llvm::StringRef Input(Buf.data(), Buf.size() - 1);

    if(Input.getAsInteger(0, Level))
      llvm::report_fatal_error("Cannot parse cache level file");
  }

  void ParseCacheKind(llvm::StringRef CachePath, HardwareCache::Kind &Kind) {
    llvm::SmallString<32> TypePath;
    llvm::sys::path::append(TypePath, CachePath, "type");

    std::string Buf;

    if(getFileSafe(TypePath, Buf))
      llvm::report_fatal_error("Cannot read cache type file");

    llvm::StringRef Input(Buf.data(), Buf.size() - 1);

    Kind = llvm::StringSwitch<HardwareCache::Kind>(Input)
           .Case("Instruction", HardwareCache::Instruction)
           .Case("Data", HardwareCache::Data)
           .Case("Unified", HardwareCache::Unified)
           .Default(HardwareCache::Invalid);
  }

  void ParseCacheSharedCPUs(llvm::StringRef CachePath,
                            HardwareComponent::LinksContainer &SharedCPUs) {
    llvm::SmallString<32> SharedCPUPath;
    llvm::sys::path::append(SharedCPUPath, CachePath, "shared_cpu_list");

    std::string Buf;

    if(getFileSafe(SharedCPUPath, Buf))
      llvm::report_fatal_error("Cannot read cache shared CPUs file");

    llvm::StringRef Input(Buf.data(), Buf.size() - 1);

    llvm::SmallVector<llvm::StringRef, 4> SharedIDs;
    Input.split(SharedIDs, ",");

    if (SharedIDs.back().size() == 0) SharedIDs.pop_back();

    for(llvm::SmallVector<llvm::StringRef, 4>::iterator I = SharedIDs.begin(),
                                                        E = SharedIDs.end();
                                                        I != E;
                                                        ++I) {
      llvm::SmallVector<llvm::StringRef, 2> RangeIDs;
      I->split(RangeIDs, "-");
      if (RangeIDs.back().size() == 0) RangeIDs.pop_back();

      assert(RangeIDs.size() <= 2 && !RangeIDs.empty());

      unsigned BeginCoreID, EndCoreID;
      if(RangeIDs.front().getAsInteger(0, BeginCoreID))
        llvm::report_fatal_error("Cannot parse cache shared CPUs file");
      if(RangeIDs.back().getAsInteger(0, EndCoreID))
        llvm::report_fatal_error("Cannot parse cache shared CPUs file");

      for (unsigned CoreID = BeginCoreID; CoreID <= EndCoreID; ++CoreID) {
        llvm::FoldingSetNodeID ID;
        void *InsertPoint;

        ID.AddInteger(CoreID);
        HardwareCPU *CPU = CPUs.FindNodeOrInsertPos(ID, InsertPoint);

        if(!CPU) {
          CPU = new HardwareCPU(CoreID);
          CPUs.InsertNode(CPU, InsertPoint);
        }

        SharedCPUs.insert(CPU);
      }
    }
  }

  void ParseCacheSize(llvm::StringRef CachePath, size_t &Size) {
    unsigned long long N;

    llvm::SmallString<32> SizePath;
    llvm::sys::path::append(SizePath, CachePath, "size");

    std::string Buf;

    if(getFileSafe(SizePath, Buf))
      llvm::report_fatal_error("Cannot read cache size file");

    const char *S = Buf.data(),
               *E = Buf.data() + Buf.size(),
               *J;

    // Look for multiplier start.
    for(J = S; J != E && std::isdigit(*J); ++J) { }

    llvm::StringRef RawN(S, J - S);
    llvm::StringRef Multiplier(J, E - 1 - J);

    if(RawN.getAsInteger(0, N))
      llvm::report_fatal_error("Cannot parse cache size file");

    N *= llvm::StringSwitch<unsigned long long>(Multiplier)
           .Case("K", 1024)
           .Default(0);

    if(!N)
      llvm::report_fatal_error("Cannot parse cache size file");

    Size = N;
  }

  void ParseCacheLineSize(llvm::StringRef CachePath, size_t &Size) {
    unsigned long long N;

    llvm::SmallString<32> LineSizePath;
    llvm::sys::path::append(LineSizePath, CachePath, "coherency_line_size");

    std::string Buf;

    if(getFileSafe(LineSizePath, Buf))
      llvm::report_fatal_error("Cannot read cache line size file");

    llvm::StringRef Input(Buf.data(), Buf.size() - 1);

    if(Input.getAsInteger(0, N))
      llvm::report_fatal_error("Cannot parse cache line size file");

    Size = N;
  }

private:
  Hardware::CPUComponents &CPUs;
  Hardware::CacheComponents &Caches;
  Hardware::NodeComponents &Nodes;
};

} // End anonymous namespace.

//
// HardwareComponent implementation.
//

HardwareComponent *HardwareComponent::GetAncestor() {
  HardwareComponent *Comp, *Anc;

  for(Comp = this, Anc = NULL; Comp; Comp = Comp->GetParent())
    Anc = Comp;

  return Anc;
}

//
// HardwareCPU implementation.
//

HardwareComponent *HardwareCPU::GetParent() {
  return GetFirstLevelCache();
}

HardwareCache *HardwareCPU::GetFirstLevelCache() const {
  for(HardwareComponent::iterator I = begin(), E = end(); I != E; ++I) {
    HardwareComponent *Comp = *I;

    if(HardwareCache *Cache = llvm::dyn_cast<HardwareCache>(Comp))
      return Cache;
  }

  return NULL;
}

//
// HardwareCache implementation.
//

HardwareComponent *HardwareCache::GetNextLevelMemory() {
  for(HardwareComponent::iterator I = begin(), E = end(); I != E; ++I) {
    if(HardwareNode *Node = llvm::dyn_cast<HardwareNode>(*I))
      return Node;

    if(HardwareCache *Cache = llvm::dyn_cast<HardwareCache>(*I))
      if(Cache->GetLevel() > Level)
        return Cache;
  }

  return NULL;
}

//
// HardwareNode implementation.
//

const HardwareCPU &HardwareNode::cpu_front() const {
  return *cpu_begin();
}

const HardwareCPU &HardwareNode::cpu_back() const {
  const_cpu_iterator I = cpu_begin(),
                     E = cpu_end(),
                     J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

const HardwareCache &HardwareNode::llc_front() const {
  return *llc_begin();
}

const HardwareCache &HardwareNode::llc_back() const {
  const_llc_iterator I = llc_begin(),
                     E = llc_end(),
                     J = I;

  for(; I != E; ++I)
    J = I;

  return *J;
}

const HardwareCache &HardwareNode::l1c_front() const {
  HardwareCPU &CPU = const_cast<HardwareCPU &>(cpu_front());

  return *llvm::cast<HardwareCache>(CPU.GetParent());
}

const HardwareCache &HardwareNode::l1c_back() const {
  HardwareCPU &CPU = const_cast<HardwareCPU &>(cpu_back());

  return *llvm::cast<HardwareCache>(CPU.GetParent());
}

unsigned HardwareNode::CPUsCount() const {
  unsigned N = 0;

  for(const_cpu_iterator I = cpu_begin(), E = cpu_end(); I != E; ++I)
    N++;

  return N;
}

//
// Hardware implementation.
//

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
  Hardware::CPUComponents CPUs;
  Hardware::CacheComponents Caches;
  Hardware::NodeComponents Nodes;

  LinuxHardwareParser Parser(CPUs, Caches, Nodes);

  Parser();
  
  for(Hardware::CPUComponents::iterator I = CPUs.begin(),
                                        E = CPUs.end();
                                        I != E;
                                        ++I)
    Components.insert(&*I);
  for(Hardware::CacheComponents::iterator I = Caches.begin(),
                                          E = Caches.end();
                                          I != E;
                                          ++I)
    Components.insert(&*I);
  for(Hardware::NodeComponents::iterator I = Nodes.begin(),
                                         E = Nodes.end();
                                         I != E;
                                         ++I)
    Components.insert(&*I);
}

Hardware::~Hardware() {
  llvm::DeleteContainerPointers(Components);
}

llvm::ManagedStatic<Hardware> HW;

Hardware &opencrun::sys::GetHardware() {
  return *HW;
}
