
//extern unsigned long int test(void);
extern unsigned long int test(unsigned long int *vars);

#if 0
unsigned long int test(void) {
  // return 0;
  return 42;
}
#endif

#if 0
unsigned long int test(unsigned long int *vars) {
  return vars[42];
}
#endif

#if 1
unsigned long int test(unsigned long int *vars) {
  __asm (
    "mov    $0x1234,%rax\n"
    "mov    $0x1234,%rbx\n"
    "mov    $0x1234,%rdi\n"
    "mov    $0x1234,%r8\n"
    "mov    $0x1234,%r15\n"

    "mov    0x1234(%rdi),%rax\n"
    "mov    0x1234(%rdi),%rcx\n"
    "mov    0x1234(%rdi),%rdx\n"
    "mov    0x1234(%rdi),%rdi\n"
    "mov    0x1234(%rdi),%r8\n"
    "mov    0x1234(%rdi),%r15\n"

    "mov    %rax, %rax\n"
    "mov    %rax, %rcx\n"
    "mov    %rax, %rdx\n"
    "mov    %rax, %rdi\n"
    "mov    %rax, %r8\n"
    "mov    %rax, %r15\n"

    "mov    %rax, %rax\n"
    "mov    %rcx, %rax\n"
    "mov    %rdx, %rax\n"
    "mov    %rdi, %rax\n"
    "mov    %r8, %rax\n"
    "mov    %r15, %rax\n"

    "mov    %r15, %r15\n"

    "add    $0x1234,%rax\n"
    "add    $0x1234,%rcx\n"
    "add    $0x1234,%rdx\n"
    "add    $0x1234,%rdi\n"
    "add    $0x1234,%r8\n"
    "add    $0x1234,%r15\n"

    "mov    0x150(%rax),%rax\n"
    "mov    0x150(%rdi),%rax\n"
    "mov    0x150(%rax),%rdx\n"
    "mov    0x150(%rdi),%rbx\n"
    "mov    0x150(%rdi),%rcx\n"
    "mov    0x150(%rdi),%rdx\n"
    "mov    0x150(%rdi),%rbx\n"
    "mov    0x150(%r8),%rax\n"
    "mov    0x150(%rax),%r8\n"
  );
  return 0;
}
#endif
