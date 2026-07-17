#define STACK_SIZE 2097152 // 2MB stack size 
#define PAGE_SIZE 4096  // page size


extern void *jstack(); // returns a stack (of size STACK_SIZE) pointer with stack protection pages on both ends of the stack ( each of size PAGE_SIZE). The returned pointer is the usable stack pointer (after the first guard page) growing upwards.
