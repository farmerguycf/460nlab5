/*
    Name 1: guy farmer
    UTEID 1: gcf375
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N - Lab 5                                           */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         pagetable    page table in LC-3b machine language   */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,COND1, COND0, COND2,        //Cond0	Cond1 Cond2
    J5, J4, J3, J2, J1, J0,     //J0 J1 J2 J3 J4 J5
    LD_MAR,                     //LD.MAR
    LD_MDR,                     //LD.MDR
    LD_IR,                      //LD.IR
    LD_BEN,                     //LD.BEN
    LD_REG,                     //LD.REG
    LD_CC,                      //LD.CC
    LD_PC,                      //LD.PC
    GATE_PC,                    //GatePC
    GATE_MDR,                   //GateMDR
    GATE_ALU,                   //GateALU
    GATE_MARMUX,                //GateMARMUX
    GATE_SHF,                   //GateSHF
    PCMUX1, PCMUX0,             //PCMUX1 PCMUX0 
    DRMUX, DRMUX1,             //DRMUX0 DRMUX1
    SR1MUX, SR1MUX1,           //SR1MUX0 SR1MUX1
    ADDR1MUX,                   //ADDR1MUX
    ADDR2MUX1, ADDR2MUX0,       //ADDR2MUX1, ADDR2MUX0
    MARMUX,                     //MARMUX
    ALUK1, ALUK0,               //ALUK1 ALUK0
    MIO_EN,                     //
    R_W,                        //
    DATA_SIZE,                  //
    LSHF1,                      //
/* MODIFY: you have to add all your new control signals */
    ld_vec,                     //
    ld_psr,                     //
    vec_mux0,                   //
    vec_mux1,                   //
    gate_sp,                    //
    sp_mux0,                    //
    sp_mux1,                    //
    gate_pc_2,                  //
    psr_mux,                    //      
    gate_psr,                   //
    ld_usp,                     //
    ld_ssp,                     // 
    gate_vec,                   //
    op_ex,                      //
    ld_cmp,                     //
    sel_vec,                    //
    ld_exc,                     //
    sto,
    restore,
    ld_seq,
    ld_vmar,
    gate_vmar,
    gate_vaddr,
    ld_pte,
    gate_pte,
    ld_par,
    st_rw,
    pte_mux,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x2000 /* 32 frames */ 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

/* For lab 4 */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int SSP; /* Initial value of system stack pointer */
/* MODIFY: you should add here any other registers you need to implement interrupts and exceptions */
int PSRReg;
int USPReg;
int protection;
int unaligned;
int opcode;
int interrupt;
int page_fault;


/* For lab 5 */
int PTBR; /* This is initialized when we load the page table */
int VA;   /* Temporary VA register */
/* MODIFY: you should add here any other registers you need to implement virtual memory */
int sequencer;
int RW;
int PAR;
int PTE;

    
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename, int is_virtual_base) {                   
    FILE * prog;
    int ii, word, program_base, pte, virtual_pc;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    if (is_virtual_base) {
      if (CURRENT_LATCHES.PTBR == 0) {
	printf("Error: Page table base not loaded %s\n", program_filename);
	exit(-1);
      }

      /* convert virtual_base to physical_base */
      virtual_pc = program_base << 1;
      pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) | 
	     MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

      printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
		if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
	      program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
   	   printf("physical base of program: %x\n\n", program_base);
	      program_base = program_base >> 1; 
		} else {
   	   printf("attempting to load a program into an invalid (non-resident) page\n\n");
			exit(-1);
		}
    }
    else {
      /* is page table */
     CURRENT_LATCHES.PTBR = program_base << 1;
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0 && is_virtual_base) 
      CURRENT_LATCHES.PC = virtual_pc;

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine         */
/*                                                             */
/***************************************************************/
void initialize(char *argv[], int num_prog_files) { 
    int i;
    init_control_store(argv[1]);

    init_memory();
    load_program(argv[2],0);
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(argv[i + 3],1);
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

/* MODIFY: you can add more initialization code HERE */

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 4) {
	printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv, argc - 3);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.
   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/


int cycleCount, memPath, pcRes, addrRes, marmuxRes, mdrRes, aluRes, shfRes, interrupt;
void eval_micro_sequencer() {

   int *microInst = CURRENT_LATCHES.MICROINSTRUCTION;
   if(CYCLE_COUNT == 300){
     CURRENT_LATCHES.interrupt = 1;
     NEXT_LATCHES.interrupt = 1;
   }
   if(CYCLE_COUNT == 0){
     CURRENT_LATCHES.PSRReg = 0x8000;
     CURRENT_LATCHES.USPReg = 0xfe00;
   }
   CURRENT_LATCHES.INTV = NEXT_LATCHES.INTV =  0x0200;
   
   int cond1 = CURRENT_LATCHES.MICROINSTRUCTION[1];
   int cond0 = CURRENT_LATCHES.MICROINSTRUCTION[2];
   int cond2 = CURRENT_LATCHES.MICROINSTRUCTION[3];
   int ben = CURRENT_LATCHES.BEN;
   int r = CURRENT_LATCHES.READY;
   if(CURRENT_LATCHES.STATE_NUMBER == 18){
     interrupt = CURRENT_LATCHES.interrupt;
     NEXT_LATCHES.interrupt = 0;
   }
 
   
   int ir11 = (CURRENT_LATCHES.IR & 0x00000800)>>11;
   int j0 = CURRENT_LATCHES.MICROINSTRUCTION[J0] || (ir11 && cond0 && cond1);
   int j1 = (CURRENT_LATCHES.MICROINSTRUCTION[J1] || (r && cond0 && !cond1))<<1;
   int j2 = (CURRENT_LATCHES.MICROINSTRUCTION[J2] || (ben && !cond0 && cond1))<<2;
   int j3 = (CURRENT_LATCHES.MICROINSTRUCTION[J3] || (cond2 && interrupt))<<3;
   int j4 = CURRENT_LATCHES.MICROINSTRUCTION[J4]<<4;
   int j5 = (CURRENT_LATCHES.MICROINSTRUCTION[J5])<< 5;
   int jReg = j0 | j1 | j2 | j3 | j4 | j5;
   int ird = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
   int instruction = (CURRENT_LATCHES.IR & 0x0000f000)>>12;
    if(microInst[restore]){
            jReg = Low16bits(CURRENT_LATCHES.sequencer);
           for(int i = 0; i< CONTROL_STORE_BITS;i++){
         NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[jReg][i];
      }
         NEXT_LATCHES.STATE_NUMBER = jReg;
       }
    else if (ird == 0){
       // if a store is initiated, we must store the jbits
       if(microInst[sto]){
           NEXT_LATCHES.sequencer = jReg;
           for(int i = 0; i< CONTROL_STORE_BITS;i++){
         NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[26][i];
      }
         NEXT_LATCHES.STATE_NUMBER = 26;
       }
       // if not, we need to go to the next state specified by the jbits
       else{
           for(int i = 0; i< CONTROL_STORE_BITS;i++){
         NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[jReg][i];
       }
         NEXT_LATCHES.STATE_NUMBER = jReg;
       }
       
   }
   else{
      for(int i = 0; i< CONTROL_STORE_BITS;i++){
	 NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[instruction][i];
      }   
         NEXT_LATCHES.STATE_NUMBER = instruction;
   }


}

void cycle_memory() {
 
   int* microInst = CURRENT_LATCHES.MICROINSTRUCTION;
   int mioen = GetMIO_EN(microInst);
   int ldMdr = GetLD_MDR(microInst);
   int write = GetR_W(microInst);
   int dataSize = GetDATA_SIZE(microInst);
   int mar0 = CURRENT_LATCHES.MAR & 0x0001;
   int we0 = (!mar0) && write;
   int we1 = write && (mar0 ^ dataSize);
   if(mioen){
     if(mioen && cycleCount<5){
       cycleCount++;
       if(cycleCount == 4){
         NEXT_LATCHES.READY = 1; 
       }
     }
     if(CURRENT_LATCHES.READY){
       if(write){
         if(we0){MEMORY[CURRENT_LATCHES.MAR >> 1][0] = Low16bits(CURRENT_LATCHES.MDR & 0x00ff);}
         if(we1){MEMORY[CURRENT_LATCHES.MAR >> 1][1] = Low16bits((CURRENT_LATCHES.MDR & 0xff00) >> 8);}
       }else{
            memPath = Low16bits(MEMORY[CURRENT_LATCHES.MAR >> 1][0] + 
              (MEMORY[CURRENT_LATCHES.MAR >> 1][1] << 8));
        }
        NEXT_LATCHES.READY = 0;
        cycleCount = 0;        
     }
   }else{
        cycleCount = 0;
        we0 = 0;
        we1 = 0;
    }

}

int psrRes, vecReg, spRes, pc_2Out, vaddrRes, pteRes, paddrRes;
void eval_bus_drivers() {

   int* microInst = CURRENT_LATCHES.MICROINSTRUCTION;
   pcRes = CURRENT_LATCHES.PC;

   //first mux
   int mux1;
   if(GetADDR1MUX(microInst)){
    int source = Low16bits((CURRENT_LATCHES.IR & 0x01c0) >> 6);
    mux1 = Low16bits(CURRENT_LATCHES.REGS[source]);
   }else{mux1 = Low16bits(CURRENT_LATCHES.PC);}

   //second mux
   int mux2;
   if(GetADDR2MUX(microInst)==0){mux2 = 0;}
   else if(GetADDR2MUX(microInst)==1){
        int ir = CURRENT_LATCHES.IR & 0x003f;
        if(ir & 0x0020){ir = Low16bits(ir | 0xffc0);}
        if(GetLSHF1(microInst)){mux2 = Low16bits(ir<<1);}
        else{mux2 = Low16bits(ir);}
   }
   else if(GetADDR2MUX(microInst)==2){
        int ir = CURRENT_LATCHES.IR & 0x01ff;
        if(ir & 0x0100){ir = Low16bits(ir | 0xfe00);}
        if(GetLSHF1(microInst)){mux2 = Low16bits(ir<<1);}
        else{mux2 = Low16bits(ir);}
 
   }
   else if(GetADDR2MUX(microInst)==3){
        int ir = CURRENT_LATCHES.IR & 0x07ff;
        if(ir & 0x0400){ir = Low16bits(ir | 0xf800);}
        if(GetLSHF1(microInst)){mux2 = Low16bits(ir<<1);}
        else{mux2 = Low16bits(ir);}
   }
   addrRes = mux1+mux2;

  // marmuxRes
  if(GetMARMUX(microInst)){marmuxRes = addrRes;}
  else{
      marmuxRes = Low16bits((CURRENT_LATCHES.IR & 0x00ff)<<1);
  }

  //mdrRes
  if(GetDATA_SIZE(microInst)){mdrRes = Low16bits(CURRENT_LATCHES.MDR);}
  else{
      if(CURRENT_LATCHES.MAR & 0x0001){
        int data = (CURRENT_LATCHES.MDR & 0xff00)>>8;
        if(data & 0x0080){mdrRes = Low16bits(data | 0xff00);}
        else{mdrRes = Low16bits(data);}
      }
      else{
        int data = CURRENT_LATCHES.MDR & 0x00ff;
        if(data & 0x0080){mdrRes = Low16bits(data | 0xff00);}
        else{mdrRes = Low16bits(data);}
      }   
   }

  // sr2mux
  int sr2mux;
  int imm5 = CURRENT_LATCHES.IR & 0x0020;
  if(!imm5){
    int sr2 = CURRENT_LATCHES.IR & 0x0007;
    sr2mux = Low16bits(CURRENT_LATCHES.REGS[sr2]);
  }
  else{
    imm5 = CURRENT_LATCHES.IR & 0x001f;
    if(imm5 & 0x0010){imm5 = imm5 | 0xffe0;}
    sr2mux = Low16bits(imm5);
  } 
  
  //aluRes
  int sr1;
   sr1 = (CURRENT_LATCHES.IR & 0x01c0)>>6;
   if(microInst[SR1MUX1]){ sr1 = 6;}
  int firstReg = Low16bits(CURRENT_LATCHES.REGS[sr1]);
  int scndReg = Low16bits(sr2mux);
  if(GetALUK(microInst)==0){
    aluRes = Low16bits(firstReg + scndReg);
  }
  if(GetALUK(microInst)==1){
    aluRes = Low16bits(firstReg & scndReg);
  }
  if(GetALUK(microInst)==2){
    aluRes = Low16bits(firstReg ^ scndReg);
  }
  if(GetALUK(microInst)==3){
    if(!GetSR1MUX(microInst)){sr1 = Low16bits(CURRENT_LATCHES.IR >> 9) & 0x0007;}
    if(microInst[SR1MUX1]){ sr1 = 6;}
    aluRes = Low16bits(CURRENT_LATCHES.REGS[sr1]);   
  }
  //shfRes
  int shfNum = CURRENT_LATCHES.IR & 0x000f;
  if(CURRENT_LATCHES.IR & 0x0010){
    if(!(CURRENT_LATCHES.IR & 0x0020)){shfRes = Low16bits(firstReg >> shfNum);}
    else{ 
	if(firstReg & 0x8000){
          for (int i = 0; i < shfNum; i++) { firstReg = Low16bits((firstReg >> 1) | 0x8000); }
          shfRes = Low16bits(firstReg);
      	}else { shfRes = Low16bits(firstReg >> shfNum); }          
    }
  } else{shfRes = Low16bits(firstReg << shfNum);}  
  //psrRes
  if(microInst[gate_psr]){
    psrRes = Low16bits(CURRENT_LATCHES.PSRReg);
  }
  
  // vecReg res
// need to adjust the vector logic
  if(microInst[ld_vec]){
    if(interrupt){
      vecReg = CURRENT_LATCHES.INTV + (0x01 << 1);
      interrupt = 0;
    }else if(CURRENT_LATCHES.protection){
      vecReg = CURRENT_LATCHES.INTV + (0x04 << 1);
      NEXT_LATCHES.protection = 0;
    }else if(CURRENT_LATCHES.page_fault){
       vecReg = CURRENT_LATCHES.INTV + (0x02 << 1);    
    }else if(CURRENT_LATCHES.unaligned){
      vecReg = CURRENT_LATCHES.INTV + (0x03 << 1);
    }else if(microInst[op_ex]){
      vecReg = CURRENT_LATCHES.INTV + (0x05 << 1);
    }
  }
  // spRes
  if(microInst[gate_sp]){
    if(microInst[SR1MUX1]){
      if((microInst[sp_mux0] == 0) && (microInst[sp_mux1]==0)){
        spRes = Low16bits(CURRENT_LATCHES.USPReg);
      }
      if((microInst[sp_mux0] == 0) && (microInst[sp_mux1]==1)){
        spRes = Low16bits(CURRENT_LATCHES.SSP);
      }
      if((microInst[sp_mux0] == 1) && (microInst[sp_mux1]==0)){
        spRes = Low16bits(CURRENT_LATCHES.REGS[6] + 2);
      }
      if((microInst[sp_mux0] == 1) && (microInst[sp_mux1]==1)){
        spRes = Low16bits(CURRENT_LATCHES.REGS[6] - 2);
      }
    }    
  }
 // ssp / usp register loading
  if(microInst[ld_usp]){
    NEXT_LATCHES.USPReg = CURRENT_LATCHES.REGS[6];
  }
  else if(microInst[ld_ssp]){
    NEXT_LATCHES.SSP = CURRENT_LATCHES.REGS[6];
  }
  else{
    NEXT_LATCHES.USPReg = CURRENT_LATCHES.USPReg;
    NEXT_LATCHES.SSP = CURRENT_LATCHES.SSP;
  }
    
 // pc-2 res
 if(microInst[gate_pc_2]){
   pc_2Out = Low16bits(CURRENT_LATCHES.PC - 2);  
 }

if(microInst[gate_vaddr]){
    vaddrRes = Low16bits(CURRENT_LATCHES.PTBR | ((CURRENT_LATCHES.VA &0xfe00) >>8));
}

if(microInst[gate_pte]){
    int pte1 = CURRENT_LATCHES.RW <<1;
    int pte0 = !(CURRENT_LATCHES.interrupt && 0x01);
    pteRes = CURRENT_LATCHES.PTE | pte1 | pte0;
    
}
if(microInst[gate_vmar]){
    int low8 = CURRENT_LATCHES.VA & 0x01ff;
    int upper5 = CURRENT_LATCHES.PTE & 0x3e00;
    paddrRes = low8 | upper5;
}
    
    
}
// will generate virtual address for PTE


void drive_bus() {

  int* microInst = CURRENT_LATCHES.MICROINSTRUCTION;
    
    if (GetGATE_PC(microInst)) { BUS = pcRes; }                
    else if (GetGATE_ALU(microInst)) { BUS = aluRes; }        
    else if (GetGATE_MDR(microInst)) { BUS = mdrRes; }         
    else if (GetGATE_SHF(microInst)) { BUS = shfRes; }         
    else if (GetGATE_MARMUX(microInst)) { BUS = marmuxRes; }   
    else if (microInst[gate_psr]){ BUS = psrRes;}
    else if (microInst[gate_vec]){BUS = vecReg;}
    else if (microInst[gate_sp]){BUS = spRes;}
    else if (microInst[gate_pc_2]){BUS = pc_2Out;}
    else if (microInst[gate_vaddr]){BUS = vaddrRes;}
    else if (microInst[gate_pte]){BUS = pteRes;}
    else if (microInst[gate_vmar]){BUS = paddrRes;}
    else { BUS = BUS; }       

}
void flushRes() {
   pcRes = 0;
   aluRes = 0;
   mdrRes = 0;
   shfRes = 0;
   marmuxRes = 0;
   psrRes = 0;
   spRes = 0;
   pc_2Out = 0;
   BUS = 0;
   memPath = 0;
   addrRes = 0;
}


void latch_datapath_values() {

  int* microInst = CURRENT_LATCHES.MICROINSTRUCTION;

   if(GetLD_PC(microInst)){ 
     if(GetPCMUX(microInst) == 1){NEXT_LATCHES.PC = Low16bits(BUS);}
     else if(GetPCMUX(microInst)==2){NEXT_LATCHES.PC = Low16bits(addrRes);}
     else if(GetPCMUX(microInst)==0){NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);}     
   }else{NEXT_LATCHES.PC = CURRENT_LATCHES.PC;}

    // here is where we latch the virtual address into the virtual mar
    if(microInst[ld_vmar]){
        NEXT_LATCHES.VA = Low16bits(BUS);
    }else{NEXT_LATCHES.VA = CURRENT_LATCHES.VA;}
    
   if(GetLD_MAR(microInst)){
       NEXT_LATCHES.MAR = Low16bits(BUS);
   }
    
    // need to rewrite the entire exception logic
   if(microInst[ld_exc]){
       
       // first lets consider the unaligned access, which is only
       // in states 6 and 7 guaranteed
       
       int dataSize = microInst[DATA_SIZE];
       int mar0 = NEXT_LATCHES.MAR;
       int psr15 = (CURRENT_LATCHES.PSRReg >> 15) & 0x0001;
       int protection = (CURRENT_LATCHES.PTE >> 3) & 0x0001;
       int pageFault = (CURRENT_LATCHES.PTE >> 2) & 0x0001;

      CURRENT_LATCHES.unaligned = dataSize & (mar0 & 0x0001) & psr15;
       // this will only occur in states 6 and 7
       if(CURRENT_LATCHES.unaligned){
           int j5 = (CURRENT_LATCHES.MICROINSTRUCTION[J5] || CURRENT_LATCHES.unaligned)<< 5;
           NEXT_LATCHES.STATE_NUMBER = NEXT_LATCHES.STATE_NUMBER | j5;
           for(int i = 0; i< CONTROL_STORE_BITS;i++){
               NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][i];
           }
       }
      // this will only be a page fault or a protection or a page fault
      // first we will handle whether or not there is a protection
       
        // if psr15 = 1 and the protection bit is 0 (other than trap) throw an exception
       
        else if(psr15 && !protection){
           CURRENT_LATCHES.protection = !protection;
           NEXT_LATCHES.STATE_NUMBER = CURRENT_LATCHES.sequencer | 0x0020;
            for(int i = 0; i< CONTROL_STORE_BITS;i++){
                NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][i];
            }
        }
       // if it isnt unaligned or a protection error, there might be a page fault
        else if(!pageFault){
            CURRENT_LATCHES.page_fault = !pageFault;
            NEXT_LATCHES.STATE_NUMBER = 53;
            for(int i = 0; i< CONTROL_STORE_BITS;i++){
                NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][i];
            }
        }
       
       
      NEXT_LATCHES.unaligned = CURRENT_LATCHES.unaligned;
      NEXT_LATCHES.protection = CURRENT_LATCHES.protection;
      NEXT_LATCHES.page_fault = CURRENT_LATCHES.page_fault;
   }
   
   if(GetLD_MDR(microInst)){
      if(GetMIO_EN(microInst)){NEXT_LATCHES.MDR = Low16bits(memPath);}
        else{                                                              
            if(GetDATA_SIZE(microInst) == 0){
               int data = BUS & 0x00ff;
               int otherData = data << 8;
               //if(data & 0x0080){data = data | 0xff00;}
                  NEXT_LATCHES.MDR = Low16bits(data|otherData);
            }else{NEXT_LATCHES.MDR = Low16bits(BUS);}
        }
    }
    
    // here is where we load a new pte
    if(microInst[ld_pte]){
        NEXT_LATCHES.PTE = NEXT_LATCHES.MDR;
    }else{NEXT_LATCHES.PTE = CURRENT_LATCHES.PTE;}
    
    
    if(microInst[st_rw]){
        NEXT_LATCHES.RW = microInst[R_W];
    }else{NEXT_LATCHES.RW = CURRENT_LATCHES.RW;}
    
   if(GetLD_IR(microInst)){NEXT_LATCHES.IR = Low16bits(BUS);}
   else {NEXT_LATCHES.IR = CURRENT_LATCHES.IR;}

   if(GetLD_BEN(microInst)){
      int isP = (CURRENT_LATCHES.IR >>  9) & CURRENT_LATCHES.P;
      int isN = (CURRENT_LATCHES.IR >> 11) & CURRENT_LATCHES.N;
      int isZ = (CURRENT_LATCHES.IR >> 10) & CURRENT_LATCHES.Z;
        NEXT_LATCHES.BEN = (isN || isZ || isP);
    }

   if(GetLD_REG(microInst)){
       int destR = Low16bits((CURRENT_LATCHES.IR & 0x0e00) >> 9);
       if (GetDRMUX(microInst)) { destR = 7; }
       if (microInst[DRMUX1]){destR = 6;}
          CURRENT_LATCHES.REGS[destR] = Low16bits(BUS);
    }

   for(int i = 0; i < LC_3b_REGS; i++){ 
       NEXT_LATCHES.REGS[i] = CURRENT_LATCHES.REGS[i]; 
   }
    if(!microInst[sto]){NEXT_LATCHES.sequencer = CURRENT_LATCHES.sequencer;}

   if (GetLD_CC(microInst)) {
       if(BUS==0){
         NEXT_LATCHES.P = 0;
         NEXT_LATCHES.Z = 1;
	 NEXT_LATCHES.N = 0;	  
	}
       else if(!(BUS & 0x8000)) {      
            NEXT_LATCHES.P = 1;
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.N = 0;
        }
        else if (BUS & 0x8000) { 
            NEXT_LATCHES.P = 0;
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.N = 1;
        }
    }
   if(microInst[psr_mux]){
    NEXT_LATCHES.PSRReg = 0;
   }else if(microInst[ld_psr]){
      NEXT_LATCHES.PSRReg = Low16bits(BUS);
      if (BUS & 0x0001){NEXT_LATCHES.P = 1;}
      else if(BUS & 0x0002){NEXT_LATCHES.Z = 1;}
      else if(BUS & 0x0004){NEXT_LATCHES.N = 1;}
    }else{
         int PSR = (CURRENT_LATCHES.PSRReg & 0x8000) | (NEXT_LATCHES.N << 2) | (NEXT_LATCHES.Z << 1) | (NEXT_LATCHES.P);
         NEXT_LATCHES.PSRReg = Low16bits(PSR);
   }
   flushRes();
}
