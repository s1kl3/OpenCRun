
#include "opencrun/Util/DimensionInfo.h"

using namespace opencrun;

//
// DimensionInfo::DimensionInfoIterator implementation.
//

size_t DimensionInfo::DimensionInfoIterator::GetWorkGroup() const {
  const IndexesContainer &WorkGroups = Indexes.second;

  // WorkGroup = WorkGroups[0]
  size_t WorkGroup = WorkGroups.front();

  for (unsigned I = 1; I < WorkGroups.size(); ++I) {
    size_t WG_Count = 1;
    for (unsigned K = 0; K < I; ++K)
      WG_Count *= DimInfo->GetWorkGroupsCount(K);

    WorkGroup += WorkGroups[I] * WG_Count;
  }

  return WorkGroup;
}

size_t
DimensionInfo::DimensionInfoIterator::GetWorkGroupsCount(unsigned I) const {
  return DimInfo->GetWorkGroupsCount(I);
}

size_t DimensionInfo::DimensionInfoIterator::GetWorkGroup(unsigned I) const {
  if (I >= DimInfo->GetDimensions())
    return 0;

  const IndexesContainer &WorkGroups = Indexes.second;

  return WorkGroups[I];
}

unsigned DimensionInfo::DimensionInfoIterator::GetWorkDim() const {
  return DimInfo->GetDimensions();
}

size_t DimensionInfo::DimensionInfoIterator::GetGlobalSize() const {
  return DimInfo->GetGlobalWorkItems();
}

size_t DimensionInfo::DimensionInfoIterator::GetLocalSize() const {
  return DimInfo->GetLocalWorkItems();
}

size_t DimensionInfo::DimensionInfoIterator::GetGlobalSize(unsigned I) const {
  return DimInfo->GetGlobalWorkItems(I);
}

size_t DimensionInfo::DimensionInfoIterator::GetGlobalId(unsigned I) const {
  if (I >= DimInfo->GetDimensions())
    return 0;

  const IndexesContainer &Locals = Indexes.first;

  return GetWorkGroup(I) * DimInfo->GetLocalWorkItems(I) + Locals[I];
}

size_t DimensionInfo::DimensionInfoIterator::GetLocalSize(unsigned I) const {
  return DimInfo->GetLocalWorkItems(I);
}

size_t DimensionInfo::DimensionInfoIterator::GetLocalId(unsigned I) const {
  if (I >= DimInfo->GetDimensions())
    return 0;

  const IndexesContainer &Locals = Indexes.first;

  return Locals[I];
}

size_t DimensionInfo::DimensionInfoIterator::GetGlobalOffset(unsigned I) const {
  return DimInfo->GetGlobalOffset(I);
}

void DimensionInfo::DimensionInfoIterator::Dump() {
  Dump(llvm::errs());
}

void DimensionInfo::DimensionInfoIterator::Dump(llvm::raw_ostream &OS) {
  OS << **this << "\n";
}

void DimensionInfo::DimensionInfoIterator::Advance(unsigned N) {
  IndexesContainer &Locals = Indexes.first;
  IndexesContainer &WorkGroups = Indexes.second;

  unsigned Carry = N;

  // Move inside the work-group.
  for(unsigned I = 0, E = Locals.size(); I != E && Carry; ++I) {
    unsigned NewId = Locals[I] + Carry,
             DimSize = DimInfo->GetLocalWorkItems(I);

    Locals[I] = NewId % DimSize;
    Carry = NewId / DimSize;
  }

  // Move between work-groups.
  for(unsigned I = 0, E = WorkGroups.size(); I != E && Carry; ++I) {
    unsigned NewId = WorkGroups[I] + Carry,
             DimSize = DimInfo->GetWorkGroupsCount(I);

    WorkGroups[I] = NewId % DimSize;
    Carry = NewId / DimSize;
  }

  // Overflow.
  if(Carry) {
    Locals.assign(Locals.size(), 0);
    WorkGroups.assign(WorkGroups.size(), 0);

    // One work-item beyond the last one of the iteration
    // space.
    WorkGroups[0] = DimInfo->GetWorkGroupsCount(0);
  }
}

//
// DimensionInfo implementation.
//

void DimensionInfo::Dump() {
  Dump(llvm::errs());
}

void DimensionInfo::Dump(llvm::raw_ostream &OS) {
  OS << *this << "\n";
}

namespace llvm {

llvm::raw_ostream &
operator<<(llvm::raw_ostream &OS,
            DimensionInfo::DimensionInfoIterator::IndexesPair &Indexes) {
  DimensionInfo::DimensionInfoIterator::IndexesContainer &Locals =
    Indexes.first;
  DimensionInfo::DimensionInfoIterator::IndexesContainer &WorkGroups =
    Indexes.second;

  OS << "[";

  DimensionInfo::DimensionInfoIterator::IndexesContainer::iterator I, E;

  I = Locals.begin();
  E = Locals.end();

  if(I != E)
    OS << *I;

  for(++I; I != E; ++I)
    OS << ", " << *I;

  I = WorkGroups.begin();
  E = WorkGroups.end();

  if(I != E)
    OS << "|" << *I;

  for(++I; I != E; ++I)
    OS << ", " << *I;

  OS << "]";

  return OS;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, DimensionInfo &DimInfo) {
  OS << "(";

  DimensionInfo::iterator I = DimInfo.begin(), E = DimInfo.end();

  if(I != E)
    OS << *I;

  for(++I; I != E; ++I)
    OS << ", " << *I;

  OS << ")";

  return OS;
}

} // End namespace llvm.
