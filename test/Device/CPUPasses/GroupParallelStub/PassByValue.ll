
; RUN: opt -load %projshlibdir/OpenCRunCPUPasses.so \
; RUN:     -cpu-group-parallel-stub                 \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

declare void @__builtin_ocl_barrier(i64);

define void @foo(i32 %a) nounwind {
entry:
  ret void
}

; CHECK:      define void @_GroupParallelStub_foo(i8** %args) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %0 = getelementptr i8*, i8** %args, i32 0
; CHECK-NEXT:   %1 = load i8*, i8** %0
; CHECK-NEXT:   %2 = bitcast i8* %1 to i32*
; CHECK-NEXT:   %3 = load i32, i32* %2
; CHECK-NEXT:   call void @foo(i32 %3)
; CHECK-NEXT:   tail call void @__builtin_ocl_barrier(i64 0)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

!opencl.kernels = !{!0}

!0 = !{void (i32)* @foo}
