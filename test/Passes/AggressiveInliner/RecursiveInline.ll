
; RUN: opt -load %projshlibdir/OpenCRunPasses.so \
; RUN:     -aggressive-inliner                   \
; RUN:     -S -o - %s | FileCheck %s
; REQUIRES: loadable_module

define i32 @foo() #0 {
entry:
  %baz = call i32 @bar() #0
  ret i32 %baz
}

define i32 @bar() #0 {
  %yez = call i32 @yep() #0
  %add = add nsw i32 %yez, 4
  ret i32 %add
}

define i32 @yep() #0 {
  ret i32 4
}

attributes #0 = { nounwind }

; CHECK:      define i32 @foo() #0 {
; CHECK-NEXT: entry:
; CHECK-NEXT:   ret i32 8
; CHECK-NEXT: }

!opencl.kernels = !{!0}

!0 = !{i32 ()* @foo}
