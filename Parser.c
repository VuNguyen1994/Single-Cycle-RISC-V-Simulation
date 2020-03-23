#include "Parser.h"

void loadInstructions(Instruction_Memory *i_mem, const char *trace)
{
  printf("Loading trace file: %s\n", trace);

  FILE *fd = fopen(trace, "r");
  if (fd == NULL)
  {
    perror("Cannot open trace file. \n");
    exit(EXIT_FAILURE);
  }

  // Iterate all the assembly instructions
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  Addr PC = 0; // program counter points to the zeroth location initially.
  int IMEM_index = 0;
  while ((read = getline(&line, &len, fd)) != -1)
  {
    // Assign program counter
    i_mem->instructions[IMEM_index].addr = PC;

    // Extract operation
    char *raw_instr = strtok(line, " ");
    if (strcmp(raw_instr, "add") == 0 ||
        strcmp(raw_instr, "sub") == 0 ||
        strcmp(raw_instr, "sll") == 0 ||
        strcmp(raw_instr, "srl") == 0 ||
        strcmp(raw_instr, "xor") == 0 ||
        strcmp(raw_instr, "or") == 0 ||
        strcmp(raw_instr, "and") == 0)
    {
      parseRType(raw_instr, &(i_mem->instructions[IMEM_index]));
    }
    else if (strcmp(raw_instr, "ld") == 0 ||
             strcmp(raw_instr, "addi") == 0 ||
             strcmp(raw_instr, "slli") == 0 ||
             strcmp(raw_instr, "xori") == 0 ||
             strcmp(raw_instr, "srli") == 0 ||
             strcmp(raw_instr, "ori") == 0 ||
             strcmp(raw_instr, "andi") == 0 ||
             strcmp(raw_instr, "jalr") == 0)
    {
      parseIType(raw_instr, &(i_mem->instructions[IMEM_index]));
    }
    else if (strcmp(raw_instr, "sd") == 0)
    {
      parseSType(raw_instr, &(i_mem->instructions[IMEM_index]));
    }
    else if (strcmp(raw_instr, "beq") == 0 ||
             strcmp(raw_instr, "bne") == 0 ||
             strcmp(raw_instr, "blt") == 0 ||
             strcmp(raw_instr, "bge") == 0)
    {
      parseSBType(raw_instr, &(i_mem->instructions[IMEM_index]));
    }
    else if (strcmp(raw_instr, "jal") == 0)
    {
      parseUJType(raw_instr, &(i_mem->instructions[IMEM_index]));
    }
    else
    {
      printf("\033[0;31m");
      printf("ERROR: instruction \'%s\' not valid for this compiler!\n\n", raw_instr);
      printf("\033[0m");
    }
    i_mem->last = &(i_mem->instructions[IMEM_index]);
    IMEM_index++;
    PC += 4;
  }

  fclose(fd);
}

void parseRType(char *opr, Instruction *instr)
{

  instr->instruction = 0;
  unsigned opcode = 51; // all these commands use 51
  unsigned funct3 = 0;
  unsigned funct7 = 0;

  if (strcmp(opr, "add") == 0)
  {
    funct3 = 0;
    funct7 = 0;
  }
  else if (strcmp(opr, "sub") == 0)
  {
    funct3 = 0;
    funct7 = 32;
  }
  else if (strcmp(opr, "sll") == 0)
  {
    funct3 = 1;
    funct7 = 0;
  }
  else if (strcmp(opr, "srl") == 0)
  {
    funct3 = 5;
    funct7 = 0;
  }
  else if (strcmp(opr, "xor") == 0)
  {
    funct3 = 4;
    funct7 = 0;
  }
  else if (strcmp(opr, "or") == 0)
  {
    funct3 = 6;
    funct7 = 32;
  }
  else if (strcmp(opr, "and") == 0)
  {
    funct3 = 7;
    funct7 = 0;
  }

  char *reg = strtok(NULL, ", ");
  unsigned rd = regIndex(reg);

  reg = strtok(NULL, ", ");
  unsigned rs_1 = regIndex(reg);

  reg = strtok(NULL, ", ");
  reg[strlen(reg) - 1] = '\0';
  unsigned rs_2 = regIndex(reg);

  // Contruct instruction
  instr->instruction |= opcode;
  instr->instruction |= (rd << 7);
  instr->instruction |= (funct3 << (7 + 5));
  instr->instruction |= (rs_1 << (7 + 5 + 3));
  instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
  instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 5));

  // printf("R/");
  printf("%x\n", instr->instruction);
}

void parseIType(char *opr, Instruction *instr)
{

  instr->instruction = 0;
  unsigned rd;
  int opcode = 19;
  int funct3 = 0;

  if (strcmp(opr, "addi") == 0)
  {
    funct3 = 0;
  }
  else if (strcmp(opr, "ld") == 0)
  {
    opcode = 3;
    funct3 = 3;
  }
  else if (strcmp(opr, "slli") == 0)
  {
    funct3 = 1;
  }
  else if (strcmp(opr, "xori") == 0)
  {
    funct3 = 4;
  }
  else if (strcmp(opr, "srli") == 0)
  {
    funct3 = 5;
  }
  else if (strcmp(opr, "ori") == 0)
  {
    funct3 = 6;
  }
  else if (strcmp(opr, "andi") == 0)
  {
    funct3 = 7;
  }
  else if (strcmp(opr, "jarl") == 0)
  {
    opcode = 103;
    funct3 = 0;
  }

  unsigned rs_1 = 0;
  unsigned immediate = 0;

  //find immidiate based on opr
  if (strcmp(opr, "ld") == 0)
  {
    char *reg = strtok(NULL, ", ");
    rd = regIndex(reg);

    opcode = 3;
    funct3 = 3;

    reg = strtok(NULL, ", ()");
    immediate = atoi(reg);

    reg = strtok(NULL, ", ()");
    rs_1 = regIndex(reg);
  }
  else
  {
    char *reg = strtok(NULL, ", ");
    rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    immediate = atoi(reg);
  }

  instr->instruction |= opcode;
  instr->instruction |= (rd << 7);
  instr->instruction |= (funct3 << (7 + 5));
  instr->instruction |= (rs_1 << (7 + 5 + 3));
  instr->instruction |= (immediate << (7 + 5 + 3 + 5));

  // printf("I/");
  printf("%x\n", instr->instruction);
}

void parseSType(char *opr, Instruction *instr)
{

  instr->instruction = 0;
  unsigned opcode = 35;
  unsigned funct3 = 0;

  if (strcmp(opr, "sd") == 0)

  {
    funct3 = 3;
  }

  char *reg = strtok(NULL, ", ");
  unsigned rs_2 = regIndex(reg);

  // reg = strtok(NULL, ", ");
  // reg[strlen(reg) - 1] = '\0'; // reg = n(a)

  reg = strtok(NULL, "(");
  unsigned immediate = atoi(reg);

  // char *strRemainder;
  // int16_t immediate = strtol(strtok(NULL, "("), &strRemainder, 10);

  reg = strtok(NULL, ")");
  unsigned rs_1 = regIndex(reg);

  // Contruct instruction
  instr->instruction |= opcode;
  instr->instruction |= ((immediate & 31) << 7); //100101011101 & 11111 = 11101
  instr->instruction |= (funct3 << (7 + 5));
  instr->instruction |= (rs_1 << (7 + 5 + 3));
  instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
  instr->instruction |= (((immediate & 4064) >> 5) << (5 + 5 + 3 + 5 + 7)); // 100101011101 -> 000001001010

  // printf("S/");
  printf("%x\n", instr->instruction);
}

void parseSBType(char *opr, Instruction *instr)
{

  // TODO: Write SB parser
  instr->instruction = 0;
  int opcode = 99;
  int funct3 = 0;
  if (strcmp(opr, "beq") == 0)
  {
    funct3 = 0;
  }
  else if (strcmp(opr, "bne") == 0)
  {
    funct3 = 1;
  }
  else if (strcmp(opr, "blt") == 0)
  {
    funct3 = 4;
  }
  else if (strcmp(opr, "bge") == 0)
  {
    funct3 = 5;
  }

  char *reg = strtok(NULL, ", ");
  unsigned rs_1 = regIndex(reg);

  reg = strtok(NULL, ", ");
  unsigned rs_2 = regIndex(reg);

  reg = strtok(NULL, ", ");
  unsigned immediate = atoi(reg);
  // immediate = immediate >> 1;

  unsigned imm_1 = ((immediate & 15) <<1) | ((immediate & 1024) >> 10);

  unsigned imm_2 = ((immediate & 2048) >> 5) | ((immediate & 1008) >> 4);

  // Contruct instruction
  instr->instruction |= opcode;
  instr->instruction |= (imm_1 << 7);
  instr->instruction |= (funct3 << (7 + 5));
  instr->instruction |= (rs_1 << (7 + 5 + 3));
  instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
  instr->instruction |= (imm_2 << (7 + 5 + 3 + 5 + 5));

  // printf("SB/");
  printf("%x\n", instr->instruction);
}

void parseUJType(char *opr, Instruction *instr)
{

  // TODO: Write UJ parser

  instr->instruction = 0;
  unsigned opcode = 0;
  unsigned rd;
  int immediate;

  if (strcmp(opr, "jal") == 0)
  {
    opcode = 111;
  }

  char *reg = strtok(NULL, ", ");
  rd = regIndex(reg);

  reg = strtok(NULL, ", ");
  reg[strlen(reg) - 1] = '\0';

  immediate = atoi(reg);

  unsigned imm1 = (immediate >> 12) & 255;
  unsigned imm2 = (immediate >> 11) & 1;
  unsigned imm3 = (immediate >> 1) & 1023;
  unsigned imm4 = (immediate >> 20) & 1;
  unsigned final_imm = (imm4 << 19) + (imm3 << 9) + (imm2 << 8) + imm1;

  // Contruct instruction
  instr->instruction |= opcode;
  instr->instruction |= (rd << 7);
  instr->instruction |= (final_imm << (7 + 5));

  // printf("UJ/");
  printf("%x\n", instr->instruction);
}

int regIndex(char *reg)
{
  unsigned i = 0;
  for (i; i < NUM_OF_REGS; i++)
  {
    if (strcmp(REGISTER_NAME[i], reg) == 0)
    {
      break;
    }
  }

  return i;
}