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
int acc;
int temp;
UINT temp_address;
int data_address;
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
// readWord(addr)연산ㄹ을 사용해야 메모리의 값에 접근할 수 있다! -> 이걸 어떻게 정수형으로 바꿔서 처리하고 다시
// 16진수로 할 수 있는지를 모르겠다.
UINT writeWords(UINT addr, UINT data1, ...) {
	va_list ap;
	UINT data;

	va_start(ap, data1);
	for (data = data1; data != END_OF_ARG; data = va_arg(ap, UINT)) {
		writeWord(addr, data);
		//printf("writeWords내의 addr 데이터 찍어보기 : %04X\n", addr);
		//printf("mem에 들어가는 addr 그대로 찍어보기 : %04X\n", readWord(addr));
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

	// DATA section ------------------------------example-----
	/*data_end = writeWords(data_bgn = 0x0100,
						0x000C,
						0x8019,
						0x0000,
						0x000A,
						0x583D,
						0x0000,
						END_OF_ARG);*/

	//homework section
	data_end = writeWords(data_bgn =
			0x0100,		0x0007,	// 0100: A=7
						0x8005,	// 0102: B=-5
						0x0000,	// 0104: X
						0x000A,	// 0106: TEN=10
						//여기서부터 Y시작
						0x0000, // 0108:Y
						0x4158, // 010A : AX,
						0x5E32,// 010C : ^2
						0x2B42,// 010E : +B
						0x582B,// 0110 : X+
						0x433D,// 0112 : C 
						//0x3D00,// 0114 : =
						0x0000,// 0116 : '\n'
						

						//0x583D,	// 0108: STR 'X' '='
						//0x0000,	// 010A:     '\0'
						END_OF_ARG);

	// CODE section ----------------------------------------

	/*code_end = writeWords(code_bgn =
			0x0200,			0x1100,	// 		0200: LDA A
						0x7102,	// 		 MUL B
						0x2104,	// 		 STA X
						0x3106,	// 		 ADD TEN
						0x2104,	// 		 STA X
						0xD108,	// 		 PRS STR
						0xB104,	// 		 PRT X
						0xC00A,	// 		 PRC '\n'
						0x8000,	// 		 HLT
						END_OF_ARG);*/

	code_end = writeWords(code_bgn = 0x0200,
						0x1106, //		LDA X
						0x7106, //		MUL X
						0x7100, //		MUL A
						0x2108, //		STA Y
						0x1106, //		LDA X
						0x7102, //		MUL B
						0x3108, //		ADD Y
						0x2108, //		STA Y
						0x1104, //		LDA C
						0x3108, //		ADD Y
						0x2108, //		STA Y,
						//0xD10A,
						//0xD10C,
						//0xD10E,
						//0xD110,
						//0xD112,
						//0xD114,
						0xD10A,
						//0x203D,
						0xB108, //		PRT Y
						0xC116, //		PRC '\n'
						0x8000, //		HLT
						END_OF_ARG);

	// -----------------------------------------------------

	// print memory for verify
	printMemory("DATA", data_bgn, data_end);
	printMemory("CODE", code_bgn, code_end);

	return code_bgn;	// return start address of program
}

//========================================
// Keyboard input for specific variables
//========================================
void inputData() {
	// print problem summary
	printf("Y = AX^2 +BX + C\n");
	
	//example
	//inputNumber("0100: A = ", 0x0100);
	//inputNumber("0102: B = ", 0x0102);
	
	// input data_homework
	inputNumber("0100: A = ", 0x0100);
	inputNumber("0102: B = ", 0x0102);
	inputNumber("0104: C = ", 0x0104);
	inputNumber("0106: X = ", 0x0106);

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

void debug_fetch(UINT pc, char ir[])
{
	printf("\n<fetch> PC:%04X IR:%s ", pc, ir);
	
}
void debug_exec(int acc)
{
	printf(" <Exec> ACC:%04X\n", cint2accnum(acc));
}

//========================================
// Run program
// - addr: start address of program
// - return exit state = 0: normal exit
//                       1: error exit
//========================================
int runProgram(UINT addr) {
	
	for(int i= addr; i < code_end; i+= 2)
        {
            
	    sprintf(instruction,"%02x%02x",mem[i],mem[i+1]);
            sprintf(address, "%c%c%c", instruction[1],instruction[2],instruction[3]);
	    UINT IR_address;
	    IR_address = strtol(address, NULL, 16);

            for(int i =0; i < 4; i++)
            {
                if(instruction[i] >= 'a' && instruction[i] <= 'z')
                    instruction[i] = instruction[i] - 32;
            }
	    //debug_mode(i, instruction, IR_address);
	    //printf("\n%d\n", acc);

            if(instruction[0] == '1') //LDA
            {
                    debug_fetch(i, instruction);
		    acc = accnum2cint(readWord(IR_address));
		    //writeWord(0x0104, cint2accnum(acc));
		    debug_exec(acc);

		    printMemory(NULL, data_bgn, data_end);
            }

            else if(instruction[0] == '2') //STA
            {
		    debug_fetch(i, instruction);
		writeWord(IR_address,cint2accnum(acc));
		debug_exec(acc);
		printMemory(NULL, data_bgn, data_end);

            }

            else if(instruction[0] == '3') //ADD
            {
		    debug_fetch(i, instruction);
		temp = accnum2cint(readWord(IR_address));
		acc += temp;
		debug_exec(acc);
		printMemory(NULL, data_bgn, data_end);

            }

            else if(instruction[0] == '4') //SUB
            {
		debug_fetch(i, instruction);
		temp = accnum2cint(readWord(IR_address));
		acc -= temp;
		debug_exec(acc);
		printMemory(NULL, data_bgn, data_end);
            }

            else if(instruction[0] == '5') //JMP Pass
            {
		debug_fetch(i, instruction);
                printf("JMP처리\n");
		debug_exec(acc);
		printMemory(NULL, data_bgn, data_end);
            }

            else if(instruction[0] == '7')
            {
		    debug_fetch(i, instruction);
		temp = accnum2cint(readWord(IR_address));
		acc *= temp;
		debug_exec(acc);
		printMemory(NULL, data_bgn, data_end);
            }

            else if(instruction[0] == 'B') //PRT
            {
		    debug_fetch(i, instruction);
		prt(IR_address);
		debug_exec(acc);

		printMemory(NULL, data_bgn, data_end);

            }

            else if(instruction[0] == 'C') // PRC
            {
		debug_fetch(i, instruction);
		temp = accnum2cint(readWord(IR_address));
		prc(IR_address);
		debug_exec(acc);
		printMemory(NULL, data_bgn, data_end);
            }
            else if(instruction[0] == 'D') // PRS
            {
		    debug_fetch(i, instruction);
		prs(IR_address);
		debug_exec(acc);
		printMemory(NULL, data_bgn, data_end);

            }

            else if(strcmp(instruction,"8002") == 0)//IAC 누산기의 값 1증가
            {
		    debug_fetch(i, instruction);
                //printf("IAC처리\n");
		acc +=1;
		debug_exec(acc);
		printMemory(NULL, data_bgn, data_end);
            }

            else if(strcmp(instruction,"8000")== 0) {
                    debug_fetch(i, instruction);
		    debug_exec(acc);
		    printMemory(NULL, data_bgn, data_end);
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
	printf("     modified by 201602141 Yoo Hwanseung\n");
	printf("========================================\n");

	printf("*** Load ***\n");
	start_addr = loadProgram();

	printf("*** Input ***\n");
	inputData();

	printf("*** Run ***\n");
	exit_code = runProgram(start_addr);

	printf("*** Exit %d ***\n", exit_code);
}
