
; RUN: opt -load %projshlibdir/OpenCRunPasses.so \
; RUN:     -footprint-estimate                   \
; RUN:     -analyze                              \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

define void @foo() #0 {
entry:
  %integer = alloca i32, align 4
  %struct = alloca {i32, i32}, align 4
  %array = alloca [4 x i32], align 4
  %vector = alloca <4 x i32>, align 16
  %pointer = alloca i32*, align 4
  ret void
}

attributes #0 = { nounwind }

; CHECK: Private memory: 48 bytes (min), 1.00000 (accuracy)

!opencl.kernels = !{!0}

!0 = !{void ()* @foo}
