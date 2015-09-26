
; RUN: opt -load %projshlibdir/OpenCRunPasses.so \
; RUN:     -footprint-estimate                   \
; RUN:     -analyze                              \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

define void @foo() #0 {
entry:
  %float = alloca float, align 4
  %double = alloca double, align 8
  %x86_fp80 = alloca x86_fp80, align 16
  %fp128 = alloca fp128, align 16
  %ppc_fp128 = alloca ppc_fp128, align 16
  %x86_mmx = alloca x86_mmx, align 16
  ret void
}

attributes #0 = { nounwind }

; CHECK: Private memory: 54 bytes (min), 1.00000 (accuracy)

!opencl.kernels = !{!0}

!0 = !{void ()* @foo}
