
; RUN: opt -load %projshlibdir/OpenCRunPasses.so \
; RUN:     -aggressive-inliner                   \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

declare float @acosf(float) #0

define float @foo() #1 {
entry:
  %acos = call float @acosf(float 0.000000e+00) #0
  ret float %acos
}

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }

; CHECK:      define float @foo() #1  {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %acos = call float @acosf(float 0.000000e+00) #0
; CHECK-NEXT:   ret float %acos
; CHECK-NEXT: }

!opencl.kernels = !{!0}

!0 = !{float ()* @foo}
