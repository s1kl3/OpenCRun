
; RUN: opt -load %projshlibdir/OpenCRunCPUPasses.so \
; RUN:     -cpu-group-parallel-stub                 \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

declare void @__builtin_ocl_barrier(i64);

define void @foo() nounwind {
entry:
  ret void
}

; CHECK:      define void @_GroupParallelStub_foo(i8** %args) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   call void @foo()
; CHECK-NEXT:   tail call void @__builtin_ocl_barrier(i64 0)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

!opencl.kernels = !{!0}

!0 = !{void ()* @foo}
