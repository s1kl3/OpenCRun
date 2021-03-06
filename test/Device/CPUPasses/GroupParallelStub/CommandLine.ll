
; RUN: opt -load %projshlibdir/OpenCRunCPUPasses.so \
; RUN:     -cpu-group-parallel-stub                 \
; RUN:     -kernel foo                              \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

define void @foo() #0 {
entry:
  ret void
}

define void @bar() #0 {
entry:
  ret void
}

attributes #0 = { nounwind }

; CHECK:      define void @foo.stub(i8** %args) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   call void @foo()
; CHECK-NEXT:   tail call void @__internal_barrier(i32 0)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK-NOT: define void @bar.stub(i8** %args)

!opencl.kernels = !{!0, !1}

!0 = !{void ()* @foo, !2}
!1 = !{void ()* @bar, !2}
!2 = !{!"custom_info"}
