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
enum registers {
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
  jit_function():
  allocated_regs(0), bytecode(NULL)
  {
  }

  // Destructor
  ~jit_function() {
    clear();
  }
  
  // Operations ========================================

  // RET; return from function
  inline void ret() {
    code.push_back(0xc3);  // ret
  }
  
  // clear return register
  inline void zero() {
    code.insert(code.end(), { 0x31, 0xc0 });  // xor    %eax,%eax
  }

  // move a specific value to a given register
  inline void move(enum registers dest, uint32_t number) {
    assert(dest != REGISTER_MAX_);

    const unsigned char* number_bytes = reinterpret_cast<unsigned char*>(&number);
    const bool extended_reg = dest >= REGISTER_R8;

    const unsigned char opcode0 = 0x48 + ( extended_reg ? 1 : 0 );
    const unsigned char opcode2 = 0xc0 + ( dest % 8 );

    code.insert(code.end(), { opcode0, 0xc7, opcode2,
                number_bytes[0], number_bytes[1], number_bytes[2], number_bytes[3] });  // mov    $0xXXXXXXXX,%YYY
  }
 
  // move a specific value to return register
  inline void move(uint32_t number) {
    return move(REGISTER_RAX, number);
  }
  
  // move a specific variable to return register
  inline void move_variable(enum registers dest, uint32_t index) {
    assert(dest != REGISTER_MAX_);

    // displacment in bytes
    index *= sizeof(uint64_t);
  
    const unsigned char* number_bytes = reinterpret_cast<unsigned char*>(&index);
    const bool extended_reg = dest >= REGISTER_R8;

    const unsigned char opcode0 = 0x48 + (extended_reg ? 4 : 0);
    const unsigned char opcode2 = 0x87 + ( dest % 8 )*8;
    
    code.insert(code.end(), { opcode0, 0x8b, opcode2,
                number_bytes[0], number_bytes[1], number_bytes[2], number_bytes[3] });  // mov    0xXXXXXXXX(%rdi),%YYY
  }
  
  // move a specific variable to return register
  inline void move_variable(uint32_t index) {
    return move_variable(REGISTER_RAX, index);
  }

  // move a specific register to another register
  inline void move(enum registers dest, enum registers src) {
    assert(dest != REGISTER_MAX_);
    assert(src != REGISTER_MAX_);

    const bool dst_extended_reg = dest >= REGISTER_R8;
    const bool src_extended_reg = src >= REGISTER_R8;

    const unsigned char opcode0 = 0x48 + ( dst_extended_reg ? 1 : 0 ) + ( src_extended_reg ? 4 : 0 );
    const unsigned char opcode2 = 0xc0 + ( dest % 8 ) + ( src % 8 )*8;

    code.insert(code.end(), { opcode0, 0x89, opcode2 });  // mov    %rXXX,%rYYY
  }
  
  // add an integer to a given register
  inline void add(enum registers dest, uint32_t number) {
    assert(dest != REGISTER_MAX_);

    const unsigned char* number_bytes = reinterpret_cast<unsigned char*>(&number);
    const bool extended_reg = dest >= REGISTER_R8;

    const unsigned char opcode0 = 0x48 + ( extended_reg ? 1 : 0 );
    const unsigned char opcode2 = 0xc0 + ( dest % 8 );

    code.insert(code.end(), { opcode0, 0x81, opcode2,
                number_bytes[0], number_bytes[1], number_bytes[2], number_bytes[3] });  // add    $0xXXXXXXXX,%rYYY
  }

  // add a register to a given register
  inline void add_reg(enum registers dest, enum registers src) {
    assert(dest != REGISTER_MAX_);
    assert(src != REGISTER_MAX_);

    const bool dst_extended_reg = dest >= REGISTER_R8;
    const bool src_extended_reg = src >= REGISTER_R8;

    const unsigned char opcode0 = 0x48 + ( dst_extended_reg ? 1 : 0 ) + ( src_extended_reg ? 4 : 0 );
    const unsigned char opcode2 = 0xc0 + ( dest % 8 ) + ( src % 8 )*8;

    code.insert(code.end(), { opcode0, 0x01, opcode2 });  // add    %rXXX,%rYYY
  }
  
  // Register allocation functions ========================================

  // allocate a register
  void new_reg(enum registers &ret) {
    size_t i;
    for(i = 0; volatile_registers[i] != REGISTER_MAX_; i++) {
      unsigned mask = 1 << volatile_registers[i];
      if ((allocated_regs & mask) == 0) {
        allocated_regs |= mask;
        ret = volatile_registers[i];
        return;
      }
    }
    abort();
    ret = REGISTER_MAX_;
  }
  
  // release a register
  void release_reg(enum registers &reg) {
    unsigned mask = 1 << reg;
    assert((allocated_regs & mask) != 0);
    allocated_regs &= ~mask;
    reg = REGISTER_MAX_;
  }
  
  // mmap/mprotect handling ========================================

  // copy code to executable location
  void build() {
    if (bytecode == NULL) {
      // Allocate one anonymous page, rw
      bytecode = mmap(NULL, page_size(), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      assert(bytecode != MAP_FAILED);

      // Copy
      assert(code.size() < page_size());
      memcpy(bytecode, &code[0], code.size());

      // Protect r+x
      if (mprotect(bytecode, page_size(), PROT_READ | PROT_EXEC) != 0) {
        abort();
      }
    }
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

  // Final execution ========================================

  // execute
  inline uint64_t operator ()(const uint64_t *const variables) {
    build();
    return function(variables);
  }

protected:
  static size_t page_size() {
    const long page = sysconf(_SC_PAGESIZE);
    assert(page > 0);
    return (size_t) page;
  }

private:
  // Volatile registers (NOT preserved across function calls)
  static const enum registers volatile_registers[];

private:
  // Forbidden foes
  jit_function(const jit_function&) = delete;
  void operator=(const jit_function&) = delete;

// Members variables
protected:
  // Allocated volatile registers
  unsigned allocated_regs;
  
  // Code array (temporary)
  std::vector<unsigned char> code;
  
  // Protected (RX) code
  union {
  void *bytecode;
  uint64_t (*function)(const uint64_t *vars);
  };
};

// Volatile registers (NOT preserved across function calls)
const enum registers jit_function::volatile_registers[] = {
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

  jit_function code;
  
#if 0
  // "return 0"
  code.clear();
  code.zero();
  code.ret();
  code.build();

  // Print result
  std::cout << "Result: " << code(variables) << "\n";
#endif
  
#if 0
  // "return 12345678;"
  code.clear();
  code.move(12345678);
  code.ret();
  code.build();
#endif

#if 0
  // return vars[VARIABLE_THREE]+42 == 3+42 == 45
  enum registers tmp ;
  code.clear();
  code.new_reg(tmp);
  code.move_variable(tmp, VARIABLE_THREE);  // tmp = vars[VARIABLE_THREE] == 3
  code.add(tmp, 42);    // tmp += 2
  code.move(REGISTER_RAX, tmp);  // return = tmp
  code.release_reg(tmp);
  code.ret();                        // return
  code.build();
#endif

#if 01
  // return vars[VARIABLE_THREE]+42+vars[VARIABLE_FOURTY_TWO] == 3+42+42 == 87
  enum registers tmp, tmp2;
  code.clear();
  code.new_reg(tmp);
  code.move_variable(tmp, VARIABLE_THREE);  // tmp = vars[VARIABLE_THREE] == 3
  code.add(tmp, 42);    // tmp += 2
  code.new_reg(tmp2);
  code.move_variable(tmp2, VARIABLE_FOURTY_TWO);  // tmp2 = var[VARIABLE_FOURTY_TWO] == 42
  code.add_reg(tmp, tmp2);  // tmp += tmp2
  code.release_reg(tmp2);
  code.move(REGISTER_RAX, tmp);  // return = tmp
  code.release_reg(tmp);
  code.ret();                        // return
  code.build();
#endif
  
  // Print result
  std::cout << "Result: " << code(variables) << "\n";
  std::cout << "Result: " << code(variables) << "\n";
  
  return EXIT_SUCCESS;
}