; RUN: opt -load %projshlibdir/OpenCRunCPUPasses.so \
; RUN:     -cpu-automatic-locals -S < %s  | FileCheck %s
; REQUIRES: loadable_module

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux"

@foo.x = internal global [10 x i32] undef, align 16
@foo.y = internal global [20 x i32] undef, align 16

define void @foo() #0 {
entry:
  call void @use(i32* getelementptr inbounds ([10 x i32], [10 x i32]* @foo.x, i32 0, i32 0))
  call void @use(i32* getelementptr inbounds ([20 x i32], [20 x i32]* @foo.y, i32 0, i32 0))
  ret void
}

declare void @use(i32*) #0

attributes #0 = { nounwind }

!opencl.kernels = !{!0}

!0 = !{void ()* @foo}

; CHECK:      define void @foo({ [10 x i32], [20 x i32] }* %automatic.locals.ptr) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %foo.x = getelementptr inbounds { [10 x i32], [20 x i32] }, { [10 x i32], [20 x i32] }* %automatic.locals.ptr, i32 0, i32 0
; CHECK-NEXT:   %0 = getelementptr inbounds [10 x i32], [10 x i32]* %foo.x, i32 0, i32 0
; CHECK-NEXT:   call void @use(i32* %0)
; CHECK-NEXT:   %foo.y = getelementptr inbounds { [10 x i32], [20 x i32] }, { [10 x i32], [20 x i32] }* %automatic.locals.ptr, i32 0, i32 1
; CHECK-NEXT:   %1 = getelementptr inbounds [20 x i32], [20 x i32]* %foo.y, i32 0, i32 0
; CHECK-NEXT:   call void @use(i32* %1)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK: !opencrun.module.info = !{!1}
; CHECK: !1 = !{!"automatic_locals", i64 120, i64 4}
