/*
 * AccCom.c - Accumulator Computer Simulator
 *
 * Created by Seokhoon Ko <shko99@gmail.com>
 * Last modified at 2021-03-13
 * You are free to modify this code for learning purposes only
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include <conio.h>

//========================================
// Global Definitions
//========================================

typedef unsigned char UCHAR;
typedef unsigned int  UINT;

#define MEM_SIZE	0x0FFF	// memory size
#define END_OF_ARG	0xFFFF	// end of argument

UCHAR mem[MEM_SIZE];	// memory image

UINT data_bgn;			// begin address of DATA section
UINT data_end;			// end address of DATA section
UINT code_bgn;			// begin address of CODE section
UINT code_end;			// end address of CODE section

int tos = 0;			// top of stack
char instruction[10];
int IR;
char address[10];
char address_to_int[10];
//========================================
// Utility Functions
// for loadProgram(), inputData()
//========================================

// Read a word data from memory
UINT readWord(UINT addr) {
	return (mem[addr] << 8) | mem[addr + 1];
}

// Write a word data to memory
void writeWord(UINT addr, UINT data) {
	mem[addr	] = (UCHAR)((data & 0xFF00) >> 8);
	mem[addr + 1] = (UCHAR) (data & 0x00FF);
}

// Write variable # of words data to memory
UINT writeWords(UINT addr, UINT data1, ...) {
	va_list ap;
	UINT data;

	va_start(ap, data1);
	for (data = data1; data != END_OF_ARG; data = va_arg(ap, UINT)) {
		writeWord(addr, data);
		addr += 2;
	}
	va_end(ap);

	return addr;	// return last address
}

// Print memory addr1 ~ (addr2 - 1)
void printMemory(char *name, UINT addr1, UINT addr2) {
	const int COL = 8;	// column size
	UINT addr;
	int c = 0;

	if (name != NULL) printf("[%s]\n",name);

	for (addr = addr1; addr < addr2; addr += 2) {
		if (c == 0) printf("%04X:", addr);
		printf(" %04X", readWord(addr));
		if (c == COL - 1) printf("\n");
		c = (c + 1)%COL;
	}
	if (c != 0) printf("\n");
}

// Convert AccCom number to C int type
int accnum2cint(UINT n) {
	UINT sign_n = n & 0x8000;	// sign of n
	UINT data_n = n & 0x7FFF;	// absolute value of n
	int i = (sign_n ? -1 : 1)*data_n;
	return i;
}

// Convert C int to AccCom number type
UINT cint2accnum(int i) {
	UINT sign_n = (UINT)((i < 0) ? 0x8000 : 0);
	UINT data_n = (UINT)abs(i) & 0x7FFF;
	UINT n = sign_n | data_n;
	return n;
}

// Scan a number and write to memory
void inputNumber(char* msg, UINT addr) {
	int n;

	printf("%s", msg);
	scanf("%d", &n);
	writeWord(addr, cint2accnum(n));
}

// stack push function
// - stack area: mem[0] ~ mem[0x00FF]
void push(UINT addr) {
	if (tos == 0x00FFFF) {
		printf("Error: Stack full");
		exit(-1);
	}
	writeWord(tos, addr);
	tos += 2;
}

// stack pop function
UINT pop() {
	if (tos == 0) {
		printf("Error: Stack empty");
		exit(-1);
	}
	tos -= 2;
	return readWord(tos);
}

//========================================
// Load AccCom program to memory
// - return start address of program
//========================================
UINT loadProgram() {
	// reset whole memory
	memset(mem, 0, MEM_SIZE);

	/*
		A=7		// input data
		B=-5	// input data
		X		// result
		10		// constant 10
		"X="	// constant "X="

		X = A*B
		X = X + 10
		print("X=", X, '\n')
	*/

	// DATA section ----------------------------------------

	data_end = writeWords(data_bgn =
			0x0100,		0x0007,	// 0100: A=7
						0x8005,	// 0102: B=-5
						0x0000,	// 0104: X
						0x000A,	// 0106: TEN=10
						0x583D,	// 0108: STR 'X' '='
						0x0000,	// 010A:     '\0'
						END_OF_ARG);

	// CODE section ----------------------------------------

	code_end = writeWords(code_bgn =
			0x0200,		0x1100,	// 0200: LDA A
						0x7102,	// 		 MUL B
						0x2104,	// 		 STA X
						0x3106,	// 		 ADD TEN
						0x2104,	// 		 STA X
						0xD108,	// 		 PRS STR
						0xB104,	// 		 PRT X
						0xC00A,	// 		 PRC '\n'
						0x8000,	// 		 HLT
						END_OF_ARG);

	// -----------------------------------------------------

	// print memory for verify
	printf("DATA 10 %d %d\n", data_bgn, data_end);
	printf("DATA 16 %02x %02x\n", data_bgn, data_end);
	printMemory("DATA", data_bgn, data_end);
	printf("CODE 10 %d %d\n", code_bgn, code_end);
	printf("CODE 16 %02x %02x\n", code_bgn, code_end);
	printMemory("CODE", code_bgn, code_end);

	return code_bgn;	// return start address of program
}

//========================================
// Keyboard input for specific variables
//========================================
void inputData() {
	// print problem summary
	printf("X = A*B + 10\n");

	// input data
	inputNumber("0100: A = ", 0x0100);
	inputNumber("0102: B = ", 0x0102);

	// print DATA section for verify
	printMemory("DATA", data_bgn, data_end);
}

//========================================
// Definitions and Functions
// for runProgram()
//========================================

// PRT (PRinT) instruction
// print a AccCom number at mem[addr]
void prt(UINT addr) {
	UINT n = readWord(addr);
	int i = accnum2cint(n);
	printf("%d", i);
}

// PRC (PRint Char) instruction
// print a ASCII char
void prc(int ch) {
	printf("%c", ch);
}

// PRS (PRint String) instruction
// print string at mem[addr]
void prs(UINT addr) {
	int ch = (int)mem[addr];
	while (ch != '\0') {
		printf("%c", ch);
		ch = (int)mem[++addr];
	}
}

//========================================
// Run program
// - addr: start address of program
// - return exit state = 0: normal exit
//                       1: error exit
//========================================
int runProgram(UINT addr) {
	int num;
	int temp;
	for(int i= addr; i < code_end; i+= 2)
        {
            sprintf(instruction,"%02x%02x",mem[i],mem[i+1]);
            sprintf(address, "%c%c%c", instruction[1],instruction[2],instruction[3]);
	    printf("address : %s\n", address);
	    num = atoi(address);
	    printf("address atoi : %d\n", num);
	    //printf("%s\n",instruction);
            for(int i =0; i < 4; i++)
            {
                if(instruction[i] >= 'a' && instruction[i] <= 'z')
                    instruction[i] = instruction[i] - 32;
            }
            printf("%s\n",instruction);
            if(instruction[0] == '1') //LDA
            {
                printf("LDA처리\n");
		temp = mem[i];
		printf("mem[i] : %d\n", mem[i]);
            }
            else if(instruction[0] == '2') //STA
            {
                printf("STA처리\n");
            }
            else if(instruction[0] == '3') //ADD
            {
                printf("ADD처리\n");
            }
            else if(instruction[0] == '4') //SUB
            {
                printf("SUB처리\n");
            }
            else if(instruction[0] == '5') //SUB
            {
                printf("JMP처리\n");
            }
            else if(instruction[0] == '7')
            {
                printf("MUL처리\n");
            }
            else if(instruction[0] == 'B')
            {
                printf("PRT처리\n");
            }
            else if(instruction[0] == 'C')
            {
                printf("PRC처리\n");
            }
            else if(instruction[0] == 'D')
            {
                printf("PRS처리\n");
            }
            else if(strcmp(instruction,"8002") == 0)
            {
                printf("IAC처리\n");
            }
            else if(strcmp(instruction,"8000")== 0) {
                printf("HLT 명령어의헤 종료 됩니다\n");
                return 0;
            }
        }
	return 0;
}

//========================================
// Main Function
//========================================
int main() {
	int exit_code;		// 0: normal exit, 1: error exit
	UINT start_addr;	// start address of program

	printf("========================================\n");
	printf(" AccCom: Accumulator Computer Simulator\n");
	printf("     modified by 201901234 Hong Gildong\n");
	printf("========================================\n");

	printf("*** Load ***\n");
	start_addr = loadProgram();

	printf("*** Input ***\n");
	//inputData();

	printf("*** Run ***\n");
	exit_code = runProgram(start_addr);

	printf("*** Exit %d ***\n", exit_code);
}