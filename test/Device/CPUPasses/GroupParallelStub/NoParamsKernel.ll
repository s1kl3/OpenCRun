
; RUN: opt -load %projshlibdir/OpenCRunCPUPasses.so \
; RUN:     -cpu-group-parallel-stub                 \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

define void @foo() nounwind {
entry:
  ret void
}

; CHECK:      define void @_GroupParallelStub_foo(i8** %args) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   call void @foo()
; CHECK-NEXT:   tail call void @__internal_barrier(i32 0)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

!opencl.kernels = !{!0}

!0 = !{void ()* @foo}
