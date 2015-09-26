
; RUN: opt -load %projshlibdir/OpenCRunPasses.so \
; RUn:     -aggressive-inliner                   \
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

; CHECK:      define void @foo() #0 {
; CHECK-NEXT: entry:
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK:      define void @bar() #0 {
; CHECK-NEXT: entry:
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

!opencl.kernels = !{!0, !1}

!0 = !{void ()* @foo}
!1 = !{void ()* @bar}
