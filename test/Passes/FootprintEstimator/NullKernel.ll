
; RUN: opt -load %projshlibdir/OpenCRunPasses.so \
; RUN:     -footprint-estimate                   \
; RUN:     -analyze                              \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

define void @foo() #0 {
entry:
  ret void
}

attributes #0 = { nounwind }

; CHECK: Private memory: 0 bytes (min), 1.00000 (accuracy)

!opencl.kernels = !{!0}

!0 = !{void ()* @foo}
