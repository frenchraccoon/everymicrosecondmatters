#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>

#define PAGE 4096

int main(int argc, char **const argv) {
  union {
    void *ptr;
    long unsigned int (*fun)(void);
  } bytecode;

  bytecode.ptr = mmap(NULL, PAGE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  assert(bytecode.ptr != MAP_FAILED);

  static const unsigned char code[] = {
    0x48, 0xc7, 0xc0, 0x2a, 0x00, 0x00, 0x00, // mov    $0x2a,%rax
    0xc3                                      // retq
  };
  memcpy(bytecode.ptr, code, sizeof(code));

  if (mprotect(bytecode.ptr, PAGE, PROT_READ | PROT_EXEC) != 0) {
    abort();
  }
  
  const long unsigned int value = bytecode.fun();

  if (munmap(bytecode.ptr, PAGE) != 0) {
    abort();
  }
  
  printf("value == %lu\n", value);

  return EXIT_SUCCESS;
}