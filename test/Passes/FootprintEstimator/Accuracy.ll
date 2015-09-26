
; RUN: opt -load %projshlibdir/OpenCRunPasses.so \
; RUN:     -footprint-estimate                   \
; RUN:     -analyze                              \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

define void @foo(i32* %out) #0 {
entry:
  %0 = alloca i32*, align 4
  store i32* %out, i32** %0, align 4
  %1 = load i32*, i32** %0, align 4
  %2 = icmp ne i32* %1, null
  br i1 %2, label %then, label %else

then:
  %3 = load i32*, i32** %0, align 4
  store i32 0, i32* %3
  br label %else

else:
  ret void
}

attributes #0 = { nounwind }

; CHECK: Private memory: 4 bytes (min), 0.30769 (accuracy)

!opencl.kernels = !{!0}

!0 = !{void (i32*)* @foo}
