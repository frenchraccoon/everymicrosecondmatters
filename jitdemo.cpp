/**
 * Let's play with code!
 **/
 
// c++ stuff
#include <vector>
#include <iostream>

// uint64_t
#include <stdint.h>

// assert, abort, sysconf, memcpy
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// mmap, mprotect
#include <sys/mman.h>

// Variables
enum {
  VARIABLE_FOURTY_TWO = 0,
  VARIABLE_ONE,
  VARIABLE_THREE,
  VARIABLE_TWO,
  VARIABLE_MAX_,
};

const uint64_t variables[VARIABLE_MAX_] = {
  [VARIABLE_FOURTY_TWO] = 42,
  [VARIABLE_ONE] = 1,
  [VARIABLE_THREE] = 3,
  [VARIABLE_TWO] = 2,
};

// x86-64 registers
enum x86_registers {
  REGISTER_RAX,
  REGISTER_RCX,
  REGISTER_RDX,
  REGISTER_RBX,
  REGISTER_RSP,
  REGISTER_RBP,
  REGISTER_RSI,
  REGISTER_RDI,
  //
  REGISTER_R8,
  REGISTER_R9,
  REGISTER_R10,
  REGISTER_R11,
  REGISTER_R12,
  REGISTER_R13,
  REGISTER_R14,
  REGISTER_R15,
  REGISTER_MAX_,
};

// Small x86-64 code wrapper
class jit_function {
public:
  // Constructor
  jit_function(const uint64_t *variables):
  variables(variables), allocated_regs(0), bytecode(NULL), function(NULL)
  {
  }

  // Destructor
  ~jit_function() {
    clear();
  }

  // RET; return from function
  inline jit_function& x86_ret() {
   code.push_back(0xc3);  // ret
   return *this;
  }
  
  // clear return register
  inline jit_function& x86_zero() {
    code.insert(code.end(), { 0x31, 0xc0 });  // xor    %eax,%eax
    return *this;
  }

  // move a specific value to a given register
  inline jit_function& x86_move(enum x86_registers dest, uint32_t number) {
    const unsigned char* number_bytes = reinterpret_cast<unsigned char*>(&number);
    const bool extended_reg = dest >= REGISTER_R8;

    const unsigned char opcode0 = 0x48 + ( extended_reg ? 1 : 0 );
    const unsigned char opcode2 = 0xc0 + ( dest % 8 );

    code.insert(code.end(), { opcode0, 0xc7, opcode2,
                number_bytes[0], number_bytes[1], number_bytes[2], number_bytes[3] });  // mov    $0xXXXXXXXX,%YYY
    return *this;
  }
 
  // move a specific value to return register
  inline jit_function& x86_move(uint32_t number) {
    return x86_move(REGISTER_RAX, number);
  }
  
  // move a specific variable to return register
  inline jit_function& x86_move_variable(enum x86_registers dest, uint32_t index) {
    // displacment in bytes
    index *= sizeof(uint64_t);
  
    const unsigned char* number_bytes = reinterpret_cast<unsigned char*>(&index);
    const bool extended_reg = dest >= REGISTER_R8;

    const unsigned char opcode0 = 0x48 + (extended_reg ? 4 : 0);
    const unsigned char opcode2 = 0x87 + ( dest % 8 )*8;

    code.insert(code.end(), { opcode0, 0x8b, opcode2,
                number_bytes[0], number_bytes[1], number_bytes[2], number_bytes[3] });  // mov    0xXXXXXXXX(%rdi),%YYY
    return *this;
  }
  
  // move a specific variable to return register
  inline jit_function& x86_move_variable(uint32_t index) {
    return x86_move_variable(REGISTER_RAX, index);
  }

  // move a specific register to another register
  inline jit_function& x86_move(enum x86_registers dest, enum x86_registers src) {
    const bool dst_extended_reg = dest >= REGISTER_R8;
    const bool src_extended_reg = src >= REGISTER_R8;

    const unsigned char opcode0 = 0x48 + ( dst_extended_reg ? 1 : 0 ) + ( src_extended_reg ? 4 : 0 );
    const unsigned char opcode2 = 0xc0 + ( dest % 8 ) + ( src % 8 )*8;

    code.insert(code.end(), { opcode0, 0x89, opcode2 });  // mov    %rXXX,%rYYY
    return *this;
  }
  
  // add an integer to a given register
  inline jit_function& x86_add(enum x86_registers dest, uint32_t number) {
    const unsigned char* number_bytes = reinterpret_cast<unsigned char*>(&number);
    const bool extended_reg = dest >= REGISTER_R8;

    const unsigned char opcode0 = 0x48 + ( extended_reg ? 1 : 0 );
    const unsigned char opcode2 = 0xc0 + ( dest % 8 );

    code.insert(code.end(), { opcode0, 0x81, opcode2,
                number_bytes[0], number_bytes[1], number_bytes[2], number_bytes[3] });  // add    $0xXXXXXXXX,%rYYY
    return *this;
  }

  // allocate a register
  enum x86_registers x86_new_reg() {
    size_t i;
    for(i = 0; volatile_registers[i] != REGISTER_MAX_; i++) {
      unsigned mask = 1 << volatile_registers[i];
      if ((allocated_regs & mask) == 0) {
        allocated_regs |= mask;
        return volatile_registers[i];
      }
    }
    abort();
  }
  
  // release a register
  void x86_release_reg(enum x86_registers reg) {
    unsigned mask = 1 << reg;
    assert((allocated_regs & mask) != 0);
    allocated_regs &= ~mask;
  }
  
  // execute
  inline uint64_t operator ()() {
    build();
    return function(variables);
  }

  // clear mapped region and vector
  void clear() {
    if (bytecode != NULL) {
      if (munmap(bytecode, page_size()) != 0) {
        abort();
      }
    }
    code.clear();
  }

protected:
  static size_t page_size() {
    const long page = sysconf(_SC_PAGESIZE);
    assert(page > 0);
    return (size_t) page;
  }

protected:
  void build() {
    if (bytecode == NULL) {
      // Allocate one anonymous page, rw
      bytecode = mmap(NULL, page_size(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      assert(bytecode != MAP_FAILED);

      // Copy
      memcpy(bytecode, &code[0], code.size());
    
      // Protect r+x
      if (mprotect(bytecode, page_size(), PROT_READ | PROT_EXEC) != 0) {
        abort();
      }
      
      // We have our function, yay!
      function = reinterpret_cast<uint64_t (*)(const uint64_t *vars)>(bytecode);
    }
  }
  
private:
  // Volatile registers (NOT preserved across function calls)
  static const enum x86_registers volatile_registers[];

private:
  // Forbidden foes
  jit_function(const jit_function&) = delete;
  jit_function& operator=(const jit_function&) = delete;

// Members variables
protected:
  // Variables array
  const uint64_t *const variables;
  
  // Allocated volatile registers
  unsigned allocated_regs;
  
  // Code array (temporary)
  std::vector<unsigned char> code;
  
  // Protected (RX) code
  void *bytecode;
  uint64_t (*function)(const uint64_t *vars);
};

// Volatile registers (NOT preserved across function calls)
const enum x86_registers jit_function::volatile_registers[] = {
  REGISTER_RAX,     // return value
  REGISTER_RCX,
  REGISTER_RDX,
  REGISTER_RSI,
  // REGISTER_RDI,  // used to pass 1st argument to functions (and we want to preserve it)
  REGISTER_R8,
  REGISTER_R9,
  REGISTER_R10,
  REGISTER_R11,
  REGISTER_MAX_,  // EOF
};
 
int main(int argc, char**const argv) {
  (void) argc;
  (void) argv;

  jit_function code(variables);
  
  // "return 0"
  //code.x86_zero().x86_ret();
  
  // "return 12345678;"
  //code.x86_move(12345678).x86_ret();
  
  // return vars[VARIABLE_THREE]+42 == 3+42 == 45
  enum x86_registers tmp = code.x86_new_reg();
  code.x86_move_variable(tmp, VARIABLE_THREE);  // tmp = 42
  code.x86_add(tmp, 42);    // tmp += 2
  code.x86_move(REGISTER_RAX, tmp);  // return = tmp
  code.x86_release_reg(tmp);
  code.x86_ret();                        // return
  
  // Print result
  std::cout << "Result: " << code() << "\n";
  
  return EXIT_SUCCESS;
}