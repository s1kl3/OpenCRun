
namespace { // Anonymous namespace.

// Very architecture-dependent code. Use __thread instead of
// llvm::sys::ThreadLocal in order to generate more compact code.
// Here we force the local-dynamic TLS access model. However,
// declaring these variables as static should be enough to lead
// to the same optimization.
//
// See <https://www.akkadia.org/drepper/tls.pdf>
static __thread uint32_t SavedEBP __attribute__((tls_model("local-dynamic"))),
                         SavedESP __attribute__((tls_model("local-dynamic"))),
                         WIStack  __attribute__((tls_model("local-dynamic")));

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
#define STACK_EBX_SAVE_OFFSET  (STACK_NEXT_OFFSET      - 4)
#define STACK_ESI_SAVE_OFFSET  (STACK_EBX_SAVE_OFFSET  - 4)
#define STACK_EDI_SAVE_OFFSET  (STACK_ESI_SAVE_OFFSET  - 4)
#define STACK_EBP_SAVE_OFFSET  (STACK_EDI_SAVE_OFFSET  - 4)
#define STACK_ESP_SAVE_OFFSET  (STACK_EBP_SAVE_OFFSET  - 4)
#define STACK_RET_SAVE_OFFSET  (STACK_ESP_SAVE_OFFSET  - 4)

#define STACK_BASE_OFFSET      (STACK_RET_SAVE_OFFSET  - 4)

#define STACK_KERN_FUNC_OFFSET (STACK_BASE_OFFSET      - 0)
#define STACK_KERN_ARGS_OFFSET (STACK_KERN_FUNC_OFFSET - 4)

#define STACK_PTR_OFFSET       (STACK_KERN_ARGS_OFFSET - 0)

// Layout notes:
//
// 0                 31
// +------------------+
// | Context          |
// | ...              |
// +------------------+
// | Kernel function  |
// | Kernel arguments |    The i386 calling conventions does not enforce 16-byte
// |                  | <- alignment, but GCC does. I think is for handling
// +------------------+    vectors. Insert padding to do the same.
// | Kernel frame     |
// | ...              |
// +------------------+
//
// Context contains next stack pointer and ebx, esi, edi, ebp, esp.

static void InitWorkItemStack(void *Stack,
                              size_t Size,
                              EntryPoint Entry,
                              void **Args) {
  uintptr_t StackAddr = (uintptr_t) Stack;

  // Stack grows downward.
  StackAddr += Size - 4;

  // Save stack info.
  *(uint32_t *) (StackAddr + STACK_EBP_SAVE_OFFSET) =
    (uint32_t) (StackAddr + STACK_BASE_OFFSET);
  *(uint32_t *) (StackAddr + STACK_ESP_SAVE_OFFSET) =
    (uint32_t) (StackAddr + STACK_PTR_OFFSET);

  // Place launcher address as fake return address.
  *(uint32_t *) (StackAddr + STACK_RET_SAVE_OFFSET) =
    (uint32_t) WorkItemLauncher;

  // Prepare arguments for launcher.
  *(uint32_t *) (StackAddr + STACK_KERN_FUNC_OFFSET) = (uint32_t) Entry;
  *(uint32_t *) (StackAddr + STACK_KERN_ARGS_OFFSET) = (uint32_t) Args;
}

static void SetWorkItemStackLink(void *Stack, void *PrevStack, size_t Size) {
  uintptr_t StackAddr = (uintptr_t) Stack,
            PrevStackAddr = (uintptr_t) PrevStack;

  // Stack grows downward.
  StackAddr += Size - 4;
  PrevStackAddr += Size - 4;

  // Link stacks.
  *(uint32_t *) (PrevStackAddr + STACK_NEXT_OFFSET) = (uint32_t) StackAddr;
}

static void RunWorkItems(void *Stack, size_t Size) {
  uintptr_t StackAddr = (uintptr_t) Stack;

  // Stack grows downward.
  StackAddr += Size - 4;

  // Save current stack address.
  __asm__ __volatile__(
  "movl %%ebp, %[SavedEBP]\n"
  "movl %%esp, %[SavedESP]\n"
  : [SavedEBP] "=c" (SavedEBP),
    [SavedESP] "=d" (SavedESP)
  :
  :
  );

  SetCurrentWorkItemStack(StackAddr);

  __asm__ __volatile__(
  // Store the values of the TLS variables into callee-saved registers in order to
  // avoid push/pop of registers before/after the kernel call.
  "movl %[SavedEBP], %%esi\n"
  "movl %[SavedESP], %%edi\n"

  // Switch to the work-item stack and directly call the kernel.
  "movl %[StackBase], %%ebp\n"
  "movl %[StackPtr], %%esp\n"
  "subl $12, %%esp\n"
  "movl -4(%%ebp), %%eax\n"
  "movl %%eax, (%%esp)\n"
  "calll *(%%ebp)\n"
  "addl $12, %%esp\n"

  // Switch back to main thread stack recovering their values from the previously
  // callee-saved registers.
  "movl %%esi, %%ebp\n"
  "movl %%edi, %%esp\n"
  :
  : [StackBase] "c" (StackAddr + STACK_BASE_OFFSET),
    [StackPtr]  "d" (StackAddr + STACK_PTR_OFFSET),
    [SavedEBP]  "m" (SavedEBP),
    [SavedESP]  "m" (SavedESP)
  : "memory", "cc"
  );

}

static void NotReachable() {
  llvm_unreachable("Work-items cannot reach here");
}

static void SetCurrentWorkItemStack(uintptr_t Stack) {
  WIStack = (uint32_t) Stack; 
}

static uintptr_t GetCurrentWorkItemStack() {
  return (uintptr_t) WIStack;
}

}

// void SwitchWorkItem()
__asm__ (
"SwitchWorkItem:\n"
// Get current stack base.
"  calll GetCurrentWorkItemStack\n"

// Save standard callee-saved registers.
"  movl %ebx, -4(%eax)\n"
"  movl %esi, -8(%eax)\n"
"  movl %edi, -12(%eax)\n"

// Save stack registers. Since we do not have a frame for this function, saving
// ebp is just a simple copy. Saving esp is more complex, because we will not
// return from this function using a ret, so we have to manually popping the
// return address from the stack.
"  movl %ebp, -16(%eax)\n"
"  movl %esp, %ebx\n"
"  addl $4, %ebx\n"
"  movl %ebx, -20(%eax)\n"

// Save return address.
"  movl (%esp), %ebx\n"
"  movl %ebx, -24(%eax)\n"

// Move to the next stack.
"  subl $12, %esp\n"
"  movl (%eax), %ebx\n"
"  movl %ebx, (%esp)\n"
"  calll SetCurrentWorkItemStack\n"
"  addl $12, %esp\n"
"  movl %ebx, %eax\n"

// Restore standard callee-saved registers.
"  movl -4(%eax), %ebx\n"
"  movl -8(%eax), %esi\n"
"  movl -12(%eax), %edi\n"

// Restore stack registers.
"  movl -16(%eax), %ebp\n"
"  movl -20(%eax), %esp\n"

// Jump to return address.
"  jmp *-24(%eax)\n"
"  calll NotReachable\n"
);

// void WorkItemLauncher()
__asm__ (
"WorkItemLauncher:\n"
"  subl $12, %esp\n"      // Make space for argument.
"  movl -4(%ebp), %eax\n" // Copy argument first in a temporary.
"  movl %eax, (%esp)\n"   // Move the temporary to call stack.
"  calll *(%ebp)\n"       // Call the kernel.
"  addl $12, %esp\n"      // Cleanup kernel parameter.
"  calll NotReachable\n"  // The previous call never returns.
);
