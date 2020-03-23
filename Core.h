#ifndef __CORE_H__
#define __CORE_H__

#include "Instruction_Memory.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define BOOL bool
#define MAX_INT 4294967296

/*Control Signal*/
struct SIGNAL{
    bool Branch, MemRead, MemtoReg, MemWrite, ALUSrc, RegWrite;
    uint8_t ALUOp;
}signal;
/*Binary Instruction Element*/
struct inst_element
{
    unsigned rs1;
    unsigned rs2;
    unsigned rd;
    unsigned opcode;
    uint64_t im;
    unsigned f3;
    unsigned f7;
}bininstruction;


struct Core;
typedef struct Core Core;
typedef struct Core
{
    Tick clk; // Keep track of core clock
    Addr PC; // Keep track of program counter

    uint64_t register_file[32];
    uint8_t data_memory[256]; 
    Instruction_Memory *instr_mem;
    
    // TODO, simulation function
    bool (*tick)(Core *core);
}Core;

Core *initCore(Instruction_Memory *i_mem);
bool tickFunc(Core *core);

void seperate(unsigned instruction);
void controlsignal(unsigned opcode);
uint64_t mux (bool signal, uint64_t input1, uint64_t input2);
unsigned getALUControl();
uint64_t convert64 (uint64_t imm);
unsigned ALU(int64_t input1, int64_t input2, unsigned ALU_Control);
bool config_branch(unsigned input1, unsigned input2);

#endif
