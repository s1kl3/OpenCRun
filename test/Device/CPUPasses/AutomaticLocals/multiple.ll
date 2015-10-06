; RUN: opt -load %projshlibdir/OpenCRunCPUPasses.so \
; RUN:     -cpu-automatic-locals -S < %s  | FileCheck %s
; REQUIRES: loadable_module

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux"

@bar.x = internal global [10 x i32] undef, align 16
@bar.y = internal global [20 x i32] undef, align 16
@foo.z = internal global i32 undef, align 4

define void @bar() #0 {
entry:
  call void @use(i32* getelementptr inbounds ([10 x i32], [10 x i32]* @bar.x, i32 0, i32 0))
  call void @use(i32* getelementptr inbounds ([20 x i32], [20 x i32]* @bar.y, i32 0, i32 0))
  ret void
}

declare void @use(i32*) #0

define void @baz() #0 {
entry:
  call void @bar()
  ret void
}

define void @taz() #0 {
entry:
  ret void
}

define void @foo() #0 {
entry:
  call void @baz()
  call void @taz()
  call void @use(i32* @foo.z)
  ret void
}

define void @root() #0 {
entry:
  call void @foo()
  call void @baz()
  ret void
}

attributes #0 = { nounwind }

!opencl.kernels = !{!0, !1, !2}

!0 = !{void ()* @bar}
!1 = !{void ()* @foo}
!2 = !{void ()* @root}

; CHECK:      define void @bar({ [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %bar.x = getelementptr inbounds { [10 x i32], [20 x i32], i32 }, { [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr, i32 0, i32 0
; CHECK-NEXT:   %0 = getelementptr inbounds [10 x i32], [10 x i32]* %bar.x, i32 0, i32 0
; CHECK-NEXT:   call void @use(i32* %0)
; CHECK-NEXT:   %bar.y = getelementptr inbounds { [10 x i32], [20 x i32], i32 }, { [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr, i32 0, i32 1
; CHECK-NEXT:   %1 = getelementptr inbounds [20 x i32], [20 x i32]* %bar.y, i32 0, i32 0
; CHECK-NEXT:   call void @use(i32* %1)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK:      define void @baz({ [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   call void @bar({ [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK-NOT: define void @taz({ [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr)

; CHECK:      define void @foo({ [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   call void @baz({ [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr)
; CHECK-NEXT:   call void @taz()
; CHECK-NEXT:   %foo.z = getelementptr inbounds { [10 x i32], [20 x i32], i32 }, { [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr, i32 0, i32 2
; CHECK-NEXT:   call void @use(i32* %foo.z)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK:      define void @root({ [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   call void @foo({ [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr)
; CHECK-NEXT:   call void @baz({ [10 x i32], [20 x i32], i32 }* %automatic.locals.ptr)
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; CHECK: !opencrun.module.info = !{!3}
; CHECK: !3 = !{!"automatic_locals", i64 124, i64 4}
