
//extern unsigned long int test(void);
extern unsigned long int test(unsigned long int *vars);

#if 0
unsigned long int test(unsigned long int *vars) {
  return vars[42] + 42;
  //return vars[0]*(vars[1]+vars[2]+1);
}
#endif

#if 0
unsigned long int test(void) {
  //return 0;
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

    "mov    0x12345678(%rdi),%rax\n"
    "mov    0x12345678(%rdi),%rcx\n"
    "mov    0x12345678(%rdi),%rdx\n"
    "mov    0x12345678(%rdi),%rdi\n"
    "mov    0x12345678(%rdi),%r8\n"
    "mov    0x12345678(%rdi),%r15\n"

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

    "add    %rax, %rax\n"
    "add    %rcx, %rax\n"
    "add    %rdx, %rax\n"
    "add    %rdi, %rax\n"
    "add    %r8, %rax\n"
    "add    %r15, %rax\n"

    "add    %rax, %rax\n"
    "add    %rax, %rcx\n"
    "add    %rax, %rdx\n"
    "add    %rax, %rdi\n"
    "add    %rax, %r8\n"
    "add    %rax, %r15\n"

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
