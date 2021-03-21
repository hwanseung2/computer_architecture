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
#include <curses.h>

// #include <conio.h> osx, linux에서는 conio.h 인클루드 안된다.

//========================================
// Global Definitions
//========================================

typedef unsigned char UCHAR;
typedef unsigned int  UINT;

#define MEM_SIZE	0x0FFF	// memory size
#define END_OF_ARG	0xFFFF	// end of argument

UCHAR mem[MEM_SIZE];	// main memory image

UINT data_bgn;			// begin address of DATA section
UINT data_end;			// end address of DATA section
UINT code_bgn;			// begin address of CODE section
UINT code_end;			// end address of CODE section

int tos = 0;			// top of stack stack 사용

//========================================
// Utility Functions
// for loadProgram(), inputData()
//========================================

// Read a word data from memory
UINT readWord(UINT addr) { // 메모리 주소를 읽기
	return (mem[addr] << 8) | mem[addr + 1];
}

// Write a word data to memory
// write Word 이 func가 mar, mbr의 역할을 한다고 생각하자
void writeWord(UINT addr, UINT data) {
	mem[addr	] = (UCHAR)((data & 0xFF00) >> 8);
	mem[addr + 1] = (UCHAR) (data & 0x00FF); // 홀수 짝수 나눧ㄴ 이유는?
}

// Write variable # of words data to memory
// ...은 가변인자
UINT writeWords(UINT addr, UINT data1, ...) { 
	va_list ap;
	UINT data;

	va_start(ap, data1);
	for (data = data1; data != END_OF_ARG; data = va_arg(ap, UINT)) {
		writeWord(addr, data);
		addr += 2; // 이 역할이 지금 PC의 역할인 듯 하다.
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
// 이 함수는 컨버트, acc의 넘버를 바꿔주는 역할 C에서 사용하는 자료형을 사용하기 위해서
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
	printf("10진수 cint2accnum n: %d\n", n);
	printf("16진수 cint2accnum n : %x\n", n);
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
// 위에는 스택영역을 정의해두심
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
// 이게 많이 중요
// 이 안에 데이터가 저장돼 있고, 코드가 저장돼있다. 명령어 프로그램이 저장돼 있다. 어셈블리 
// 인풋데이터에서는 내가 필요한 부분만 ㅇ받을 수 있다.
//
UINT loadProgram() {
	// reset whole memory
	memset(mem, 0, MEM_SIZE);

	/*
		A=12	// input data
		B=-25	// input data
		X		// result
		10		// constant 10
		"X="	// constant "X="

		X = A + B
		X = X + 10
		print("X=", X, '\n')
	*/

	// DATA section ----------------------------------------
	// 지금 data_end에 들어가는 값이 임의의 숫자들이지만 내가 넣는 값이 여기로 들어가야하는 것이 아닌가?
	// 예를들어 A에 672를 넣는다면 그것이 16진수로 변환 돼서 들어가야함이 맞지않나?
	// 명령어 리스트를 본다고 하면

	// 0 : NCP(아무일도 하지 않는다.)
	// 1 : LDA(메모리에 저장된 값을 누산기에 적재)
	// 2 : STA(누산기의 값을 메모리에 저장)
	// 3 : ADD(누산기의 값에 메모리의 값을 더하기)
	// 4 : SUB(누산기의 값에서 메모리의 값을 빼기)
	// 5 : JMP(지정한 주소로 분기)
	// 7 : MUL(누산기의 값에 메모리의 값을 곱하기)
	// 8000 : HLT(프로그램 종료)
	// 8002 : IAC(누산기의 값을 1 증가)
	// 8005 : RET(프로시저 복귀)

	data_end = writeWords(data_bgn =
			0x0100,			0x000C,	// 0100: A=12 / C가 가리키는 게 12임
						0x8019,	// 0102: B=-25
						0x0000,	// 0104: X
						0x000A,	// 0106: 10
						0x583D,	// 0108:	"X=" {'X', '='}
						0x0000,	// 010A:	     {'\0'}
						END_OF_ARG);

	// CODE section ----------------------------------------

	/*code_end = writeWords(code_bgn =
			0x0200,			0x1100,	// 0200:	LDA A
						0x7102,	// 		MUL B
						0x2104,	// 		STA X
						0x3106,	// 		ADD 10
						0x2104,	// 		STA X
						0xD108,	// 		PRS "X="
						0xB104,	// 		PRT X
						0xC00A,	// 		PRC '\n'
						0x8000,	// 		HLT
						END_OF_ARG);
	*/
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
						0xD108, //		PRS "Y="
						0xB104, //		PRT Y
						0xC00A, //		PRC '\n'
						0x8000, //		HLT
						END_OF_ARG);
	// -----------------------------------------------------

	// print memory for verify
	printf("DATA의 시작과 끝 address : %x, %x : ", data_bgn, data_end);
	printf("\n");
	printf("CODE의 시작과 끝 address : %x, %x : ", code_bgn, code_end);
	printf("\n");
	printMemory("DATA", data_bgn, data_end);
	printMemory("CODE", code_bgn, code_end);

	return code_bgn;	// return start address of program
}

//========================================
// Keyboard input for specific variables
//========================================
void inputData() {
	// print problem summary
	printf("Y = AX^2 + BX + C\n");

	// input data
	inputNumber("0100: A = ", 0x0100);
	inputNumber("0102: B = ", 0x0102);
	inputNumber("0104: C = ", 0x0104);
	inputNumber("0106: X = ", 0x0106);
	

	// print DATA section for verify
	printf("DATA에서 data_bgn, data_end찍기 : %04X, %04X : ", data_bgn, data_end);
	printf("\n");
	printMemory("DATA", data_bgn, data_end);
}

//========================================
// Definitions and Functions
// for runProgram()
//========================================

// 여기에 내가 작성을 해야한다!!!!!

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

// PRS (PRint String) nstruction
// print string at mem[addr]
void prs(UINT addr) {
	int ch = (int)mem[addr];
	while (ch != '\0') {
		printf("%c", ch);
		ch = (int)mem[++addr];
	}
} // 스트링 출력한다. 백슬래시 제로가 나올때까지

//========================================
// Run program
// - addr: start address of program
// - return exit state = 0: normal exit
//                       1: error exit
//========================================
int runProgram(UINT addr) {
	printf("UINT address : %d\n", addr);
	for(int i=addr;i<addr+30;i+=2)
	{
		printf("memory[i] : %x ", mem[i]); //아 여기서 지금 11이랑 00 이렇게 출력되는 것이 11, 00에서 인트로 출력받기 때문에 0으로 출력되는거구나 그렇다면 아예 address를 다룰 때, i는 2씩 증가해야한다!
		printf("memory[i+1] : %x", mem[i+1]);
		printf("\n");
		printf("명령어만 끌고오는 정수 뽑아보기 : %1x ", mem[i]);
		printf("\n");
	}
	printf("메모리의 100번지와 102번지에 데이터가 들어가는 걸로 아는데 그걸 한번 찍어보자\n");
	printf("mem[100, 102출력] : %x, %x", mem[100], mem[102]); // 00으로 나온다.
	printf("\n");
	return 0;
	//정상적으로 halt를 만나면 0을 리턴하라.
}

//========================================
// Main Function
//========================================
// MAR, MBR : WriteWord
// 명령어는 어디서 받는가? runprogram인가
// PC는 mem이 맞나
// decode 하는 과정
// IR우리가 짜야하냐?
// LDA, ADD ㅇ이거 우리가짜야하겠네요
int main() {
	int exit_code;		// 0: normal exit, 1: error exit
	UINT start_addr;	// start address of program
	printf("start address : %d\n", start_addr);

	printf("========================================\n");
	printf(" AccCom: Accumulator Computer Simulator\n");
	printf("     modified by 201602141 유환승\n");
	printf("========================================\n");

	printf("*** Load ***\n");
	start_addr = loadProgram();

	printf("*** Input ***\n");
	inputData();

	printf("*** Run ***\n");
	exit_code = runProgram(start_addr); //매개변수로 시작주소를 알려준다.

	printf("*** Exit %d ***\n", exit_code);
}

