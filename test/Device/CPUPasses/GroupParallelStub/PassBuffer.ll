
; RUN: opt -load %projshlibdir/OpenCRunCPUPasses.so \
; RUN:     -cpu-group-parallel-stub                 \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

define void @foo(i32* nocapture %a) nounwind {
entry:
  ret void
}

; CHECK:      define void @foo.stub(i8** %args) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %0 = getelementptr i8*, i8** %args, i32 0
; CHECK-NEXT:   %1 = bitcast i8** %0 to i32**
; CHECK-NEXT:   %2 = load i32*, i32** %1
; CHECK-NEXT:   call void @foo(i32* %2)
; CHECK-NEXT:   tail call void @__internal_barrier(i32 0)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

!opencl.kernels = !{!0}

!0 = !{void (i32*)* @foo, !1}
!1 = !{!"custom_info"}
