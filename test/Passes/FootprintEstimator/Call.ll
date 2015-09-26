
; RUN: opt -load %projshlibdir/OpenCRunPasses.so \
; RUN:     -footprint-estimate                   \
; RUN:     -analyze                              \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

declare float @acosf(float) #0 readnone

define void @foo() #0 {
entry:
  %acos = call float @acosf(float 0.000000e+00) #0 readnone
  ret void
}

attributes #0 = { nounwind }

; CHECK: Private memory: 0 bytes (min), 0.00000 (accuracy)

!opencl.kernels = !{!0}

!0 = !{void ()* @foo}
