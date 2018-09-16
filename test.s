 mov    0x10(%rdi),%rax
 add    0x8(%rdi),%rax
 add    $0x1,%rax
 imul   (%rdi),%rax
 retq
