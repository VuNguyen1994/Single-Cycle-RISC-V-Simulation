#include "Core.h"
#include "Registers.h"

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;
    int i;
    /*Default init - DO NOT CHANGE */
    core->register_file[0] = 0;
    for (i = 1; i < 32; i++) { 
        core->register_file[i] = 0;
    }
    for(int i = 0; i < 16; i++)
	    core->data_memory[i*8] = 1;

    /*Init register value, memory value FOR REPORT*/
    core->register_file[25] = 4;
    core->register_file[10] = 4;
    core->register_file[22] = 1;
    
    core->data_memory[0] = 16;
    core->data_memory[8] = 128;
    core ->data_memory[16]= 8;
    core->data_memory[24] = 4;
    

    /* Init values for MATRIX
    for (int k = 0; k < 128; k+=8)
    {
        core->data_memory[k] = k/8; 
    }*/
    
    return core;
}

/*Control functions*/
/*Check for negative imm + Convert from 12bits to 64 bits*/
uint64_t convert64 (uint64_t imm)
{
    uint64_t imm64 = imm;
    if (bininstruction.opcode == 111) //UJ type - 20 bits
    {
        if (imm & 0x80000)
        {
            imm64= imm | 0xfff00000;
            imm64 -= MAX_INT; //convert to negative signed number
        }
    }
    else //regular opcode 12 bits
    {
        if (imm & 0x800){ //if im negative
            imm64= imm | 0xfffff000;
            imm64 -= MAX_INT;
        } 
    }
    return imm64;
}

/*Seperate the fields of binary instruction*/
void seperate(unsigned instruction){
    /*Init the field of the parsed binary instruction*/
    bininstruction.opcode = (instruction << 25) >> 25; //opcode always the last 7 bits
    bininstruction.rs1 = 0;
    bininstruction.rs2 = 0;
    bininstruction.rd = 0;
    bininstruction.im = 0;
    bininstruction.f3 = 0;
    bininstruction.f7 = 0;
    if (bininstruction.opcode == 51) // R type
    {
        bininstruction.f3 = (instruction << 17) >> 29;
        bininstruction.f7 = (instruction >> 25);
        bininstruction.rs1 = (instruction << 12) >> 27;
        bininstruction.rs2 = (instruction << 7) >> 27; 
        bininstruction.rd = (instruction << 20 ) >> 27;
    }
    else if (bininstruction.opcode == 19) // I type
    {
        bininstruction.rs1 = (instruction << 12) >> 27;
        bininstruction.im = instruction >> 20; //left 12 digit
        bininstruction.im = convert64(bininstruction.im);
        bininstruction.f3 = (instruction << 17) >> 29;
        bininstruction.rd = (instruction << 20 ) >> 27;
    }
    else if(bininstruction.opcode == 3) // I type, ld
    {
        bininstruction.im = instruction >> 20;
        bininstruction.im = convert64(bininstruction.im);
        bininstruction.f3 = (instruction << 17) >> 29;
        bininstruction.rs1 = (instruction << 12) >> 27;
        bininstruction.rd = (instruction << 20 ) >> 27;
    }
    else if (bininstruction.opcode == 103) // I type, jarl
    {
        bininstruction.rs1 = (instruction << 12) >> 27;
        bininstruction.im = instruction >> 20; //left 12 digit
        bininstruction.im = convert64(bininstruction.im);
        bininstruction.f3 = (instruction << 17) >> 29;
        bininstruction.rd = (instruction << 20 ) >> 27;
    }
    else if (bininstruction.opcode == 35) // S type, sd
    {
        bininstruction.im = ((instruction >> 25)<<5) | ((instruction << 20)  >> 27);
        bininstruction.im = convert64(bininstruction.im);
        bininstruction.f3 = (instruction << 17) >> 29;
        bininstruction.rs1 = (instruction << 12) >> 27;
        bininstruction.rs2 = (instruction << 7) >> 27; 
    }
    else if (bininstruction.opcode == 99) //SB type
    {
        //buil im for sb
        unsigned i12, i11, i10_5, i4_1;
        i12 = ((instruction >> 31) <<11);
        i11 =  (((instruction << 24)>>31) << 10);
        i10_5 = (((instruction << 1) >> 26)<<4);
        i4_1 = ((instruction <<20) >> 28);
        bininstruction.im = (i12 | i11 | i10_5 | i4_1);
        bininstruction.im = convert64(bininstruction.im);
        bininstruction.f3 = (instruction << 17) >> 29;
        bininstruction.rs1 = (instruction << 12) >> 27;
        bininstruction.rs2 = (instruction << 7) >> 27; 
    }
    else if (bininstruction.opcode == 111) //UJ type, jal
    {
        //build im for uj type
        unsigned i20, i10_1, i11, i19_12;
        i20 = (instruction >> 31) << 19;
        i10_1 = ((instruction << 1) >> 22);
        i11 = ((instruction << 11) >> 31) << 10;
        i19_12 = ((instruction << 12) >> 24) << 11;
        bininstruction.im = i20 | i19_12 | i11 | i10_1;
        bininstruction.im = convert64(bininstruction.im);
        bininstruction.rd = (instruction << 20) >> 27;
    }
    else
    {
        printf("Seperate Error: Opcode Unfound!\n");
    }
}
/*Config branch signal bne, beq, blt OR bge*/
bool config_branch(unsigned input1, unsigned input2)
{    
    if (bininstruction.opcode == 99){
        switch(bininstruction.f3){
            
            case 0: //beq
                if (input1== input2)
                    return true;
                else
                {
                    return false;
                }
                
                break;
                
            case 1: //bne
                if (input1 != input2)
                {
                    return true;
                }
                else
                {
                    return false;
                }
                
                break;
                
            case 4: //blt
                if (input1 <= input2)
                {
                    return true;
                }
                else
                {
                    return false;
                }
                
                break;
                
            case 5: //bge
                if (input1 >= input2)
                    return true;
                else
                {
                    return false;
                }
                
                break;
                
            default: 
                return false;
                break;
        }
    }
    if (bininstruction.opcode == 111 || bininstruction.opcode == 103)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*Config signals in system*/
void controlsignal(unsigned opcode){
    /*Init signals as false and 0 so it is reset for each call for each instruction */
    signal.ALUSrc = false;
    signal.MemtoReg = false;
    signal.RegWrite = false;
    signal.MemRead = false;
    signal.MemWrite = false;
    signal.Branch = false;
    signal.ALUOp = 0;
    
    if (opcode == 51) // R type
    {
        signal.RegWrite = true;
        signal.ALUOp = 2;
    }
    else if (opcode == 19) // I type
    {
        signal.ALUSrc = true;
        signal.RegWrite = true;
        signal.ALUOp = 2;
    }
    else if(opcode == 3) // I type, ld
    {
        signal.ALUSrc = true;
        signal.MemtoReg = true;
        signal.RegWrite = true;
        signal.MemRead = true;
        signal.ALUOp = 0;
    }
    else if (opcode == 103) // I type, jarl
    {
        signal.ALUSrc = true;
        signal.RegWrite = true;
        signal.Branch = true;
        signal.ALUOp = 0;
    }
    else if (opcode == 35) // S type, sd
    {
        signal.ALUSrc = true;
        signal.MemWrite = true;
        signal.ALUOp = 0;
    }
    else if (opcode == 99) //SB type, bne, beq, blt, bge
    {
        signal.Branch = true;
        signal.ALUOp = 1;
    }
    else if (opcode == 111) //UJ type, jal
    {
        signal.ALUSrc = true;
        signal.RegWrite = true;
        signal.Branch = true;
        signal.ALUOp = 0;
    }
    else
    {
        printf("Error: Undefined opcode!\n");
    }
}

/*ALU Operations*/
unsigned ALU(int64_t input1, int64_t input2, unsigned ALU_Control)
{   
    unsigned final_output = 0;
    switch(ALU_Control){
            
        case 0: 
            final_output = input1 & input2;
            break;
        case 1: 
            final_output = input1 | input2;
            break;            
        case 2: 
            final_output = input1 + input2;
            break;
        case 3: 
            final_output = input1 << input2;
            break;
        case 4: 
            final_output = input1 >> input2;
            break;
        case 5: 
            final_output = input1 ^ input2;
            break;
        case 6: 
            final_output = input1 - input2;
            break;
        case 7: 
            final_output = (input1 < input2 ? 1 : 0);
            break; 
        default:
            final_output = (input1 < input2 ? 1 : 0);
    }
    
    return final_output; 
}

/*Convert ALUOp to ALUControl on ALU*/
unsigned getALUControl(){ 
        // ld, sd
    if (signal.ALUOp == 0){
        return 2;
    }
    else if (signal.ALUOp == 1){
        // beq, bne
        if (bininstruction.f3 == 0 || bininstruction.f3 == 1)
            return 6;

        // blt, bge
        if (bininstruction.f3 >= 4 && bininstruction.f3 <= 5)
            return 7;
    }
    //  R-type
    else if(signal.ALUOp == 2){
        // add, addi
        if (bininstruction.f3 == 0){
            if (bininstruction.f7 == 0){
                return 2;
            }
        // sub
            else if (bininstruction.f7 == 0b0100000){
                return 6; 
            }
        }
        // sll, slli
        if (bininstruction.f3 == 1){
            return 3;
        }
        // slt
        if (bininstruction.f3 == 2){
            return 7;
        }
        // xor, xori
        if (bininstruction.f3 == 4){
            return 5;
        }
        // srl, srli
        if (bininstruction.f3 == 5){
            return 4;
        }
        // or, ori
        if (bininstruction.f3 == 6){
            return 1;
        }
        // and, andi
        if (bininstruction.f3 == 7){
            return 0;
        }
    }
    else {
        printf("Can't retrieve ALUControl: Unknown ALUOp Signal!\n");
        return -1;
    }
    return -1;
}

/*Muxes*/
uint64_t mux (bool signal, uint64_t input1, uint64_t input2){
    if (signal){
        return input1;
    }
    else
    {
        return input2;
    }
}


// FIXME, implement this function
bool tickFunc(Core *core)
{
    // (Step 1) Reading instruction from instruction memory
    unsigned instruction = 0;
    instruction = core->instr_mem->instructions[core->PC / 4].instruction;
   
    // (Step 2) 
    /*Separate the binary instruction received into opcode, rd, rs and immediate*/
    seperate(instruction);
    /*Init the control signals*/
    controlsignal(bininstruction.opcode);

    int64_t read_dat1 = core->register_file[bininstruction.rs1];
    int64_t read_dat2 = core->register_file[bininstruction.rs2];
    
    unsigned alucontrol_signal = getALUControl();
    read_dat2 = mux(signal.ALUSrc, bininstruction.im, read_dat2); //imm or rs2
    unsigned result = ALU(read_dat1, read_dat2, alucontrol_signal);
    
    // (Step 3) Signal Handler for Memory and register file 
    int64_t r_dat = 0;
    int w_dat;

    if(signal.MemWrite) //Only sd has MemWrite
    {
        r_dat = core->register_file[bininstruction.rs2];
        for(int i = 0; i < 64; i+=8)
            core->data_memory[result+i/8] = r_dat << i;
    }

    if(signal.MemRead)
    {
        for(int i = 0; i < 64; i+=8)
            r_dat |= core->data_memory[result+i/8] << i;
    }

    if(signal.MemtoReg)
    {
        w_dat = r_dat;
    }
    else if(bininstruction.opcode == 103 /*jal*/ || bininstruction.opcode == 111 /*jalr*/)
    {
        w_dat = core->PC + 4;
    }
    else
    {
        w_dat = result;
    }

    if(bininstruction.rd != 0 && signal.RegWrite)
    {
        core->register_file[bininstruction.rd] = w_dat;
    }
    
    // (Step 4) Config PC 
    unsigned new_PC = core->PC + bininstruction.im; //True for all cases except jalr
    if(bininstruction.opcode == 103 /*jalr*/)
    {
        new_PC = result; //RS1+imm
    }


    signal.Branch = config_branch(read_dat1, read_dat2); //update signal.Branch
    
    if(signal.Branch)/*bne, beq, blt, bge, jal, jalr*/
    {
        core->PC = new_PC;
    }
    else
    {
        core->PC += 4;
    }

    // (Step 5) Output
    /*Output Format*/
    printf("\nInstruction (Hex): %x\n", instruction);
    printf("Op: %u\t rd: %u\t rs1: %u\t rs2: %u \timm: %ld \tresult: %d\n", bininstruction.opcode, bininstruction.rd, bininstruction.rs1, bininstruction.rs2, bininstruction.im, result);
    printf("REGISTER VALUE (NON-ZERO): \n");
    for(int i = 0; i < 32 ; i++) //# of regs is 32
    {
        if (core->register_file[i] != 0) //Print out only registers with nonzero value
        {
            printf("%s: %lu\n", REGISTER_NAME[i], core->register_file[i]);
        }
    }

    printf("DATA MEMORY: \n");
    printf("Address  |   Data\n");
    for(int i = 0; i < 256; i += 8) //#of byte in mem is 256 //8
    {
	    int dat = 0;
        int j = 0;
	    while(j < 7)
	    {
	        dat |= (core->data_memory[i+j] << (j * 8));
            j++;
	    }
        
	    printf("%4d     |   %u\n", i, dat);
    } 
    

    
    // (Step 6)
    ++core->clk;
    /* Are we reaching the final instruction?*/
    if (core->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}
