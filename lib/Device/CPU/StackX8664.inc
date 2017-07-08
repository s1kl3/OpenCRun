
namespace { // Anonymous namespace.

// Very architecture-dependent code. Use __thread instead of
// llvm::sys::ThreadLocal in order to generate more compact code.
// Here we force the local-dynamic TLS access model. However,
// declaring these variables as static should be enough to lead
// to the same optimization.
//
// See <https://www.akkadia.org/drepper/tls.pdf>
static __thread uint64_t SavedRBP   __attribute__((tls_model("local-dynamic"))),
                         SavedRSP   __attribute__((tls_model("local-dynamic"))),
                         WIStack    __attribute__((tls_model("local-dynamic"))); 

} // End anonymous namespace.

extern "C" {

// Redo declarations here, I do not want to include any header.
typedef void (*EntryPoint)(void **);

void SwitchWorkItem();
void WorkItemLauncher();

static void NotReachable() __attribute__((used));

static void SetCurrentWorkItemStack(uintptr_t Stack) __attribute__((used));
static uintptr_t GetCurrentWorkItemStack() __attribute__((used));

// Stack offsets in bytes.
#define STACK_CONTEXT_OFFSET   (                         0)

#define STACK_NEXT_OFFSET      (STACK_CONTEXT_OFFSET   - 0)
#define STACK_RBX_SAVE_OFFSET  (STACK_NEXT_OFFSET      - 8)
#define STACK_R12_SAVE_OFFSET  (STACK_RBX_SAVE_OFFSET  - 8)
#define STACK_R13_SAVE_OFFSET  (STACK_R12_SAVE_OFFSET  - 8)
#define STACK_R14_SAVE_OFFSET  (STACK_R13_SAVE_OFFSET  - 8)
#define STACK_R15_SAVE_OFFSET  (STACK_R14_SAVE_OFFSET  - 8)
#define STACK_RBP_SAVE_OFFSET  (STACK_R15_SAVE_OFFSET  - 8)
#define STACK_RSP_SAVE_OFFSET  (STACK_RBP_SAVE_OFFSET  - 8)
#define STACK_RET_SAVE_OFFSET  (STACK_RSP_SAVE_OFFSET  - 8)

#define STACK_BASE_OFFSET      (STACK_RET_SAVE_OFFSET  - 8)

#define STACK_KERN_FUNC_OFFSET (STACK_BASE_OFFSET      - 0)
#define STACK_KERN_ARGS_OFFSET (STACK_KERN_FUNC_OFFSET - 8)

#define STACK_PTR_OFFSET       (STACK_KERN_ARGS_OFFSET - 8)

// Layout notes:
//
// 0                 63
// +------------------+
// | Context          |
// | ...              |
// +------------------+
// | Kernel function  |
// | Kernel arguments |
// |                  | <- Padding inserted because RSP must be 16-byte aligned
// +------------------+    before call instruction.
// | Kernel frame     |
// | ...              |
// +------------------+
//
// Context contains next stack pointer and rbx, r12, r13, r14, r15, rbp, rsp.

static void InitWorkItemStack(void *Stack,
                              size_t Size,
                              EntryPoint Entry,
                              void **Args) {
  uintptr_t StackAddr = (uintptr_t) Stack;

  // Stack grows downward.
  StackAddr += Size - 8;

  // Save stack info.
  *(uint64_t *) (StackAddr + STACK_RBP_SAVE_OFFSET) =
    (uint64_t) (StackAddr + STACK_BASE_OFFSET);
  *(uint64_t *) (StackAddr + STACK_RSP_SAVE_OFFSET) =
    (uint64_t) (StackAddr + STACK_PTR_OFFSET);

  // Place launcher address as fake return address.
  *(uint64_t *) (StackAddr + STACK_RET_SAVE_OFFSET) =
    (uint64_t) WorkItemLauncher;

  // Prepare arguments for launcher.
  *(uint64_t *) (StackAddr + STACK_KERN_FUNC_OFFSET) = (uint64_t) Entry;
  *(uint64_t *) (StackAddr + STACK_KERN_ARGS_OFFSET) = (uint64_t) Args;
}

static void SetWorkItemStackLink(void *Stack, void *PrevStack, size_t Size) {
  uintptr_t StackAddr = (uintptr_t) Stack,
            PrevStackAddr = (uintptr_t) PrevStack;

  // Stack grows downward.
  StackAddr += Size - 8;
  PrevStackAddr += Size - 8;

  // Link stacks.
  *(uint64_t *) (PrevStackAddr + STACK_NEXT_OFFSET) = (uint64_t) StackAddr;
}

static void RunWorkItems(void *Stack, size_t Size) {
  uintptr_t StackAddr = (uintptr_t) Stack;
  
  // Stack grows downward.
  StackAddr += Size - 8;

  // Save current stack addresses.
  __asm__ __volatile__(
  "movq %%rbp, %[SavedRBP]\n"
  "movq %%rsp, %[SavedRSP]\n"
  : [SavedRBP] "=m" (SavedRBP),
    [SavedRSP] "=m" (SavedRSP)
  :
  :
  );

  SetCurrentWorkItemStack(StackAddr);

  __asm__ __volatile__(
  // Store the values of the TLS variables into callee-saved registers in order to
  // avoid push/pop of registers before kernel call to prevent their corruption.
  "movq %[SavedRBP], %%r12\n"
  "movq %[SavedRSP], %%r13\n"

  // Switch to the work-item stack and directly call the kernel.
  "movq  %[StackBase], %%rbp\n"
  "movq  %[StackPtr],  %%rsp\n"
  "movq  -8(%%rbp),    %%rdi\n"
  "callq  *(%%rbp)\n"

  // Switch back to main thread stack recovering their values from the previously
  // callee-saved registers.
  "movq %%r12, %%rbp\n"
  "movq %%r13, %%rsp\n"
  :
  : [StackBase] "r" (StackAddr + STACK_BASE_OFFSET),
    [StackPtr]  "r" (StackAddr + STACK_PTR_OFFSET),
    [SavedRBP]  "m" (SavedRBP),
    [SavedRSP]  "m" (SavedRSP)
  : "memory", "cc"
  );

}

static void NotReachable() {
  llvm_unreachable("Work-items cannot reach here");
}

static void SetCurrentWorkItemStack(uintptr_t Stack) {
  WIStack = (uint64_t) Stack; 
}

static uintptr_t GetCurrentWorkItemStack() {
  return (uintptr_t) WIStack;
}

}

// void SwitchWorkItem()
__asm__ (
"SwitchWorkItem:\n"
// Get current stack base.
"  callq GetCurrentWorkItemStack\n"

// Save standard callee-saved registers.
"  movq %rbx, -8(%rax)\n"
"  movq %r12, -16(%rax)\n"
"  movq %r13, -24(%rax)\n"
"  movq %r14, -32(%rax)\n"
"  movq %r15, -40(%rax)\n"

// Save stack registers. Since we do not have a frame for this function, saving
// rbp is just a simple copy. Saving rsp is more complex, because we will not
// return from this function using a ret, so we have to manually popping the
// return address from the stack.
"  movq %rbp, -48(%rax)\n"
"  leaq 8(%rsp), %rbx\n"
"  movq %rbx, -56(%rax)\n"

// Save return address.
"  movq (%rsp), %rbx\n"
"  movq %rbx, -64(%rax)\n"

// Move to the next stack.
"  movq (%rax), %rbx\n"
"  movq %rbx, %rdi\n"
"  callq SetCurrentWorkItemStack\n"
"  movq %rbx, %rax\n"

// Restore standard callee-saved registers.
"  movq -8(%rax), %rbx\n"
"  movq -16(%rax), %r12\n"
"  movq -24(%rax), %r13\n"
"  movq -32(%rax), %r14\n"
"  movq -40(%rax), %r15\n"

// Restore stack registers.
"  movq -48(%rax), %rbp\n"
"  movq -56(%rax), %rsp\n"

// Jump to return address.
"  jmp *-64(%rax)\n"
"  callq NotReachable\n"
);

// void WorkItemLauncher()
__asm__ (
"WorkItemLauncher:\n"
"  movq -8(%rbp), %rdi\n" // Load args.
"  callq *(%rbp)\n"       // Call kernel.
"  callq NotReachable\n"  // The previous call never returns.
);
