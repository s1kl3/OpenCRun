; RUN: opt -load %projshlibdir/OpenCRunCPUPasses.so \
; RUN:     -cpu-automatic-locals -S < %s  | FileCheck %s
; REQUIRES: loadable_module

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@foo.x = internal global i32 undef, align 4
@foo.y = internal global i32 undef, align 4

; Function Attrs: nounwind uwtable
define void @foo(i32 %z, ...) #0 {
entry:
  call void @use(i32* @foo.x)
  call void @use(i32* @foo.y)
  ret void
}

declare void @use(i32*) #0

attributes #0 = { nounwind }

!opencl.kernels = !{!0}

!0 = !{void (i32, ...)* @foo}

; CHECK:      define void @foo(i32 %z, { i32, i32 }* %automatic.locals.ptr, ...) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %foo.x = getelementptr inbounds { i32, i32 }, { i32, i32 }* %automatic.locals.ptr, i32 0, i32 0
; CHECK-NEXT:   call void @use(i32* %foo.x)
; CHECK-NEXT:   %foo.y = getelementptr inbounds { i32, i32 }, { i32, i32 }* %automatic.locals.ptr, i32 0, i32 1
; CHECK-NEXT:   call void @use(i32* %foo.y)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK: !opencrun.module.info = !{!1}
; CHECK: !1 = !{!"automatic_locals", i64 8, i64 4}

