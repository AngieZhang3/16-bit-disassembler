// disassembly.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h> 
#include <ctype.h>
#include <string.h>
#define REG_AX 0
#define REG_CX 1
#define REG_DX 2
#define REG_BX 3
#define REG_SP 4
#define REG_BP 5
#define REG_SI  6
#define REG_DI 7


const char* Register16[8] = { "AX","CX" ,"DX" ,"BX" ,"SP" ,"BP" ,"SI" ,"DI" };
const char* Register8[8] = { "AL","CL" ,"DL" ,"BL" ,"AH" ,"CH" ,"DH" ,"BH" };
const char* RegisterEA[8] = { "BX + SI","BX + DI","BP + SI","BP + DI","SI","DI","BP","BX" };
const char* SegReg[4] = { "ES", "CS", "SS", "DS" };
//printf("%s", regs[src_reg]); convert table to array
//printf("%s", regs[code & 7]);


typedef struct stDisasm {

	char cAsm[60]; //output assembly code  MOV AX, BX
	int len;
	char code[60]; //store machine code
	int src_reg;
	int dst_reg;
}stDisasm;

typedef struct stModRegRm
{
	unsigned char mod : 2;
	unsigned char reg : 3;
	unsigned char rm : 3;
}stModRegRm;

stModRegRm extractModRegRm(unsigned char byte) {
	stModRegRm modRegRm;
	modRegRm.mod = (byte >> 6) & 0x03; // Extract the top 2 bits
	modRegRm.reg = (byte >> 3) & 0x07; // Extract the next 3 bits
	modRegRm.rm = byte & 0x07;        // Extract the bottom 3 bits

	return modRegRm;
}

//parse mod and r/m, return code strings
void decodeModRm(char* buffer, unsigned char mod, unsigned char rm, char* code, stDisasm* disasm)
{
	int w = code[0] & 0x01;
	switch (mod)
	{
		unsigned short addr;

	case 0x00:
		if (rm == 0x06) // 110 direct address (16 bits)
		{
			addr = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
			sprintf(buffer, "[%04X]", addr);
			disasm->len = 4;
		}
		else {
			sprintf(buffer, "[% s]", RegisterEA[rm]);
			disasm->len = 2;
		}
		break;
	case 0x01:
		sprintf(buffer, "[%s + %02X]", RegisterEA[rm], code[2]);
		disasm->len = 3;
		break;
	case 0x02:
		addr = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
		sprintf(buffer, "[%s + %04X]", RegisterEA[rm], addr);
		disasm->len = 4;
		break;
	case 0x03:
		if (w == 0)
		{
			sprintf(buffer, "%s", Register8[rm]);
		}
		if (w == 1)
		{
			sprintf(buffer, "%s", Register16[rm]);
		}

		disasm->len = 2;
		break;
	}

}





//for loop, which determines how many bytes to read next time based on the number of bytes returned. 
//Exit the loop when an error is encountered, or when parsing is finished
int Disasm(char* code, int size, /*int address, */stDisasm* disasm) {
	if (code == NULL || size <= 0 || disasm == NULL) {
		return -1;
	}

	//read and parse the first byte(OpCode)
	unsigned char firstByte = code[0];
	stModRegRm modRegRm = extractModRegRm(code[1]);
	const char* regName;
	char buffer[32];
	unsigned short data;
	const char* SR;
	switch (firstByte) {
		//const char* regName;
		//char buffer[32];
		//unsigned short data;
		//const char* SR;
	case 0x00:  //ADD
		//d = 0(reg is source) , w = 0 (byte)

		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "ADD %s, %s", buffer, regName);
		break;
	case 0x01:  //ADD 
	  //d = 0(reg is source) , w = 1 (word)
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "ADD %s, %s", buffer, regName);
		break;
	case 0x02:  //ADD
		  //d = 1(reg is destination) , w = 0 (byte)
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "ADD %s, %s", regName, buffer);
		break;
	case 0x03: //ADD 
		 //d = 1(reg is destination) , w = 1 (word)
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "ADD %s, %s", regName, buffer);
		break;
	case 0x04://ADD 
		//w = 0 immediate to accumulator al, byte 2 =  data
		sprintf(disasm->cAsm, "ADD AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x05://ADD 
		 //w = 1 immediate to accumulator AX, byte 2&3 =  data
		data = (unsigned short)(code[1]) | ((unsigned short)(code[2]) << 8);
		sprintf(disasm->cAsm, "ADD AX, %04X", data);
		disasm->len = 3;
		break;

	case 0x06: //PUSH ES
		sprintf(disasm->cAsm, "PUSH ES");
		disasm->len = 1;
		break;
	case 0x07:   // POP ES
		sprintf(disasm->cAsm, "POP ES");
		disasm->len = 1;
		break;
	case 0x08:   // OR
		//d = 0, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "OR %s, %s", buffer, regName);
		break;
	case 0x09:
		//d = 0, w = 1 OR
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "OR %s, %s", buffer, regName);
		break;
	case 0x0A:
		//d = 1, w = 0 OR
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "OR %s, %s", regName, buffer);
		break;
	case 0x0B:
		//d = 1, w = 1 OR
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "OR %s, %s", regName, buffer);
		break;
	case 0x0C: //OR
		//immdediate to AL, w= 0
		sprintf(disasm->cAsm, "OR AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x0D: //OR
		//immediate to AX, w= 1 byte 2&3 =  data
		data = (unsigned short)(code[1]) | ((unsigned short)(code[2]) << 8);
		sprintf(disasm->cAsm, "OR AX, %04X", data);
		disasm->len = 3;
		break;
	case 0x0E: //PUSH CS
		sprintf(disasm->cAsm, "PUSH CS");
		disasm->len = 1;
		break;
	case 0x10: //ADC
		//d = 0, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "ADC %s, %s", buffer, regName);
		break;
	case 0x11: //ADC
		//d = 0, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "ADC %s, %s", buffer, regName);
		break;
	case 0x12: //ADC
		//d = 1, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "ADC %s, %s", regName, buffer);
		break;
	case 0x13: //ADC
		//d = 1, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "ADC %s, %s", regName, buffer);
		break;
	case 0x14:
		//w = 0, ADC, immediate to al, byte 2 =  data
		sprintf(disasm->cAsm, "ADC AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x15:
		//w = 1, ADC, imm to AX, byte 2 & 3 = data
		data = (unsigned short)(code[1]) | ((unsigned short)(code[2]) << 8);
		sprintf(disasm->cAsm, "ADC AX, %04X", data);
		disasm->len = 3;
		break;
	case 0x16: //push ss
		sprintf(disasm->cAsm, "PUSH SS");
		disasm->len = 1;
		break;
	case 0x17: //pop ss
		sprintf(disasm->cAsm, "POP SS");
		disasm->len = 1;
		break;
	case 0x18:  //SBB
		//d = 0, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "SBB %s, %s", buffer, regName);
		break;
	case 0x19: //SBB
		//d = 0, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "SBB %s, %s", buffer, regName);
		break;
	case 0x1A:// SBB
		//d = 1, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "SBB %s, %s", regName, buffer);
		break;
	case 0x1B://SBB
		//d = 1, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "SBB %s, %s", regName, buffer);
		break;
	case 0x1C: //SBB
			   //w = 0,  immediate from al, byte 2 =  data
		sprintf(disasm->cAsm, "SBB AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x1D: //SBB
		 //w = 1,  imm from AX, byte 2 & 3 = data
		data = (unsigned short)(code[1]) | ((unsigned short)(code[2]) << 8);
		sprintf(disasm->cAsm, "SBB AX, %04X", data);
		disasm->len = 3;
		break;
	case 0x1E: //push ds
		sprintf(disasm->cAsm, "PUSH DS");
		disasm->len = 1;
		break;
	case 0x1F: //pop ds
		sprintf(disasm->cAsm, "POP DS");
		disasm->len = 1;
		break;
	case 0x20:  //AND
			//d = 0, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "AND %s, %s", buffer, regName);
		break;
	case 0x21: //AND
		//d = 0, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "AND %s, %s", buffer, regName);
		break;
	case 0x22:// AND
		//d = 1, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "AND %s, %s", regName, buffer);
		break;
	case 0x23://AND
		//d = 1, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "ADD %s, %s", regName, buffer);
		break;
	case 0x24: //AND
			   //w = 0,  immediate from al, byte 2 =  data
		sprintf(disasm->cAsm, "AND AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x25: //AND
		 //w = 1,  imm from AX, byte 2 & 3 = data
		data = (unsigned short)(code[1]) | ((unsigned short)(code[2]) << 8);
		sprintf(disasm->cAsm, "AND AX, %04X", data);
		disasm->len = 3;
		break;
	case 0x26: //ES:
		sprintf(disasm->cAsm, "ES:");
		disasm->len = 1;
		break;
	case 0x27: //DAA
		sprintf(disasm->cAsm, "DAA");
		disasm->len = 1;
		break;
	case 0x28:  //SUB
	//d = 0, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "SUB %s, %s", buffer, regName);
		break;
	case 0x29: //SUB
		//d = 0, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "SUB %s, %s", buffer, regName);
		break;
	case 0x2A:// SUB
		//d = 1, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "SUB %s, %s", regName, buffer);
		break;
	case 0x2B://SUB
		//d = 1, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "SUB %s, %s", regName, buffer);
		break;
	case 0x2C: //SUB
			   //w = 0,  immediate from al, byte 2 =  data
		sprintf(disasm->cAsm, "SUB AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x2D: //SUB
		 //w = 1,  imm from AX, byte 2 & 3 = data
		data = (unsigned short)(code[1]) | ((unsigned short)(code[2]) << 8);
		sprintf(disasm->cAsm, "SUB AX, %04X", data);
		disasm->len = 3;
		break;
	case 0x2E: //CS:
		sprintf(disasm->cAsm, "CS:");
		disasm->len = 1;
		break;
	case 0x2F: //DAS
		sprintf(disasm->cAsm, "DAS");
		disasm->len = 1;
		break;
	case 0x30:  //XOR
//d = 0, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "XOR %s, %s", buffer, regName);
		break;
	case 0x31: //XOR
		//d = 0, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "XOR %s, %s", buffer, regName);
		break;
	case 0x32:// XOR
		//d = 1, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "XOR %s, %s", regName, buffer);
		break;
	case 0x33://XOR
		//d = 1, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "XOR %s, %s", regName, buffer);
		break;
	case 0x34: //XOR
			   //w = 0,  immediate from al, byte 2 =  data
		sprintf(disasm->cAsm, "XOR AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x35: //XOR
		 //w = 1,  imm from AX, byte 2 & 3 = data
		data = (unsigned short)(code[1]) | ((unsigned short)(code[2]) << 8);
		sprintf(disasm->cAsm, "XOR AX, %04X", data);
		disasm->len = 3;
		break;
	case 0x36: //SS:
		sprintf(disasm->cAsm, "SS:");
		disasm->len = 1;
		break;
	case 0x37: //AAA
		sprintf(disasm->cAsm, "AAA");
		disasm->len = 1;
		break;
	case 0x38:  //CMP
//d = 0, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "CMP %s, %s", buffer, regName);
		break;
	case 0x39: //CMP
		//d = 0, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "CMP %s, %s", buffer, regName);
		break;
	case 0x3A:// CMP
		//d = 1, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "CMP %s, %s", regName, buffer);
		break;
	case 0x3B://CMP
		//d = 1, w = 1 
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "CMP %s, %s", regName, buffer);
		break;
	case 0x3C: //CMP
			   //w = 0,  immediate from al, byte 2 =  data
		sprintf(disasm->cAsm, "CMP AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x3D: //CMP
		 //w = 1,  imm from AX, byte 2 & 3 = data
		data = (unsigned short)(code[1]) | ((unsigned short)(code[2]) << 8);
		sprintf(disasm->cAsm, "CMP AX, %04X", data);
		disasm->len = 3;
		break;
	case 0x3E: //DS:
		sprintf(disasm->cAsm, "DS:");
		disasm->len = 1;
		break;
	case 0x3F: //AAS
		sprintf(disasm->cAsm, "AAS");
		disasm->len = 1;
		break;
	case 0x40: //INC
		sprintf(disasm->cAsm, "INC AX");
		disasm->len = 1;
		break;
	case 0x41: //INC
		sprintf(disasm->cAsm, "INC CX");
		disasm->len = 1;
		break;
	case 0x42: //INC
		sprintf(disasm->cAsm, "INC DX");
		disasm->len = 1;
		break;
	case 0x43: //INC
		sprintf(disasm->cAsm, "INC BX");
		disasm->len = 1;
		break;
	case 0x44: //INC
		sprintf(disasm->cAsm, "INC SP");
		disasm->len = 1;
		break;
	case 0x45: //INC
		sprintf(disasm->cAsm, "INC BP");
		disasm->len = 1;
		break;
	case 0x46: //INC
		sprintf(disasm->cAsm, "INC SI");
		disasm->len = 1;
		break;
	case 0x47: //INC
		sprintf(disasm->cAsm, "INC DI");
		disasm->len = 1;
		break;
	case 0x48: //DEC
		sprintf(disasm->cAsm, "DEC AX");
		disasm->len = 1;
		break;
	case 0x49: //DEC
		sprintf(disasm->cAsm, "DEC CX");
		disasm->len = 1;
		break;
	case 0x4A: //DEC
		sprintf(disasm->cAsm, "DEC DX");
		disasm->len = 1;
		break;
	case 0x4B: //DEC
		sprintf(disasm->cAsm, "DEC BX");
		disasm->len = 1;
		break;
	case 0x4C: //DEC
		sprintf(disasm->cAsm, "DEC SP");
		disasm->len = 1;
		break;
	case 0x4D: //DEC
		sprintf(disasm->cAsm, "DEC BP");
		disasm->len = 1;
		break;
	case 0x4E: //DEC
		sprintf(disasm->cAsm, "DEC SI");
		disasm->len = 1;
		break;
	case 0x4F: //DEC
		sprintf(disasm->cAsm, "DEC DI");
		disasm->len = 1;
		break;
	case 0x50: //PUSH
		sprintf(disasm->cAsm, "PUSH AX");
		disasm->len = 1;
		break;
	case 0x51: //PUSH
		sprintf(disasm->cAsm, "PUSH CX");
		disasm->len = 1;
		break;
	case 0x52: //PUSH
		sprintf(disasm->cAsm, "PUSH DX");
		disasm->len = 1;
		break;
	case 0x53: //PUSH
		sprintf(disasm->cAsm, "PUSH BX");
		disasm->len = 1;
		break;
	case 0x54: //PUSH
		sprintf(disasm->cAsm, "PUSH SP");
		disasm->len = 1;
		break;
	case 0x55: //PUSH
		sprintf(disasm->cAsm, "PUSH BP");
		disasm->len = 1;
		break;
	case 0x56: //PUSH
		sprintf(disasm->cAsm, "PUSH SI");
		disasm->len = 1;
		break;
	case 0x57: //PUSH
		sprintf(disasm->cAsm, "PUSH DI");
		disasm->len = 1;
		break;
	case 0x58: //POP
		sprintf(disasm->cAsm, "POP AX");
		disasm->len = 1;
		break;
	case 0x59: //POP
		sprintf(disasm->cAsm, "POP CX");
		disasm->len = 1;
		break;
	case 0x5A: //POP
		sprintf(disasm->cAsm, "POP DX");
		disasm->len = 1;
		break;
	case 0x5B: //POP
		sprintf(disasm->cAsm, "POP BX");
		disasm->len = 1;
		break;
	case 0x5C: //POP
		sprintf(disasm->cAsm, "POP SP");
		disasm->len = 1;
		break;
	case 0x5D: //POP
		sprintf(disasm->cAsm, "POP BP");
		disasm->len = 1;
		break;
	case 0x5E: //POP
		sprintf(disasm->cAsm, "POP SI");
		disasm->len = 1;
		break;
	case 0x5F: //POP
		sprintf(disasm->cAsm, "POP DI");
		disasm->len = 1;
		break;
	case 0x70: //JO
		sprintf(disasm->cAsm, "JO %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x71: //JNO
		sprintf(disasm->cAsm, "JNO %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x72: //JB
		sprintf(disasm->cAsm, "JB %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x73: //JAE
		sprintf(disasm->cAsm, "JAE %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x74: //JE
		sprintf(disasm->cAsm, "JE %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x75: //JNE
		sprintf(disasm->cAsm, "JNE %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x76: //JBE
		sprintf(disasm->cAsm, "JBE %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x77: //JA
		sprintf(disasm->cAsm, "JA %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x78: //JS
		sprintf(disasm->cAsm, "JS %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x79: //JNS
		sprintf(disasm->cAsm, "JNS %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x7A:  //JP
		sprintf(disasm->cAsm, "JP %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x7B:  //JNP
		sprintf(disasm->cAsm, "JNP %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x7C:  //JL
		sprintf(disasm->cAsm, "JL %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x7D:  //JGE
		sprintf(disasm->cAsm, "JGE %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x7E:  //JLE
		sprintf(disasm->cAsm, "JLE %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x7F:  //JG
		sprintf(disasm->cAsm, "JG %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0x80: //ADD if REG is 000
		//Opcode (0x80): This indicates that the next byte will define a byte operation,
		// This operation can be  ADD、SUB、AND and etc，depending on reg filed in ModR/M.
		// MOD = 11，dest. register is defined in R/M. 
	   // s = 0(no sign extension), w = 0,  immediate to register, memory
		if (modRegRm.reg == 0) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "ADD %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 1) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "OR %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 2) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "ADC %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 3) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "SBB %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 4) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "AND %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 5) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "SUB %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 6) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "XOR %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 7) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "CMP %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		break;
	case 0x81: //ADD if REG is 000
		// s = 0(no sign extension), w = 1
		if (modRegRm.reg == 0) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			data = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
			sprintf(disasm->cAsm, "ADD %s, %04X", buffer, data);
			disasm->len += 2;
		}
		if (modRegRm.reg == 1) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			data = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
			sprintf(disasm->cAsm, "OR %s, %04X", buffer, data);
			disasm->len += 2;
		}
		if (modRegRm.reg == 2) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			data = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
			sprintf(disasm->cAsm, "ADC %s, %04X", buffer, data);
			disasm->len += 2;
		}
		if (modRegRm.reg == 3) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			data = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
			sprintf(disasm->cAsm, "SBB %s, %04X", buffer, data);
			disasm->len += 2;
		}
		if (modRegRm.reg == 4) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			data = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
			sprintf(disasm->cAsm, "AND %s, %04X", buffer, data);
			disasm->len += 2;
		}
		if (modRegRm.reg == 5) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			data = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
			sprintf(disasm->cAsm, "SUB %s, %04X", buffer, data);
			disasm->len += 2;
		}
		if (modRegRm.reg == 6) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			data = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
			sprintf(disasm->cAsm, "XOR %s, %04X", buffer, data);
			disasm->len += 2;
		}
		if (modRegRm.reg == 7) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			data = (unsigned short)(code[2]) | ((unsigned short)(code[3]) << 8);
			sprintf(disasm->cAsm, "CMP %s, %04X", buffer, data);
			disasm->len += 2;
		}
		break;
	case 0x82: //ADD if REG is 000
		//s = 1 w = 0
		if (modRegRm.reg == 0) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "ADD %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 2) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "ADC %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 3) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "SBB %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 5) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "SUB %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 7) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "CMP %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		break;
	case 0x83: //ADD if REG is 000
		//s =1, w = 1 ,Note that this is always followed by an imm8
		//On execution, this 8-bit immediate number (05) is symbolically expanded to 16 or 32 bits(depending on the target size of the operation),
		//It is then added to the value in the BX register. 
		//Symbol expansion means that the highest bit of the immediate number (the sign bit) will be copied into the expanded bit to keep the sign of the numeric value unchanged.
		if (modRegRm.reg == 0) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "ADD %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 2) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "ADC %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 3) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "SBB %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 5) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "SUB %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		if (modRegRm.reg == 7) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			sprintf(disasm->cAsm, "CMP %s, %02X", buffer, (unsigned char)code[2]);
			disasm->len += 1;
		}
		break;
	case 0x84: //TEST
		//d = 0, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "TEST %s, %s", buffer, regName);
		break;
	case 0x85: //TEST
		//d = 0, w = 1
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "TEST %s, %s", buffer, regName);
		break;
	case 0x86: //XCHG
		//d = 1, w = 0
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "XCHG %s, %s", regName, buffer);
		break;
	case 0x87: //XCHG
		// d = 1,  w= 1 
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "XCHG %s, %s", regName, buffer);
		break;
	case 0x88: //MOV
		//d = 0(reg is source) , w = 0 (byte)
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "MOV %s, %s", buffer, regName);
		break;
	case 0x89:  //MOV
	  //d = 0(reg is source) , w = 1 (word)
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "MOV %s, %s", buffer, regName);
		break;
	case 0x8A:  //MOV
		  //d = 1(reg is destination) , w = 0 (byte)
		regName = Register8[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "MOV %s, %s", regName, buffer);
		break;
	case 0x8B: //MOV 
		 //d = 1(reg is destination) , w = 1 (word)
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "MOV %s, %s", regName, buffer);
		break;
	case 0x8C: //mov
		regName = Register16[modRegRm.rm];
		SR = SegReg[modRegRm.reg];
		sprintf(disasm->cAsm, "MOV %s, %s", regName, SR);
		disasm->len = 2;
		break;
	case 0x8D:
		regName = Register16[modRegRm.rm];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "LEA %s, %s", regName, buffer);
		break;
	case 0x8E:
		regName = Register16[modRegRm.rm];
		SR = SegReg[modRegRm.reg];
		sprintf(disasm->cAsm, "MOV %s, %s", SR, regName);
		disasm->len = 2;
		break;
	case 0x8F:
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "POP %s", buffer);
		break;
	case 0x90:
		sprintf(disasm->cAsm, "NOP");
		disasm->len = 1;
		break;
	case 0x91:
		sprintf(disasm->cAsm, "XCHG AX, CX");
		disasm->len = 1;
		break;
	case 0x92:
		sprintf(disasm->cAsm, "XCHG AX, DX");
		disasm->len = 1;
		break;
	case 0x93:
		sprintf(disasm->cAsm, "XCHG AX, BX");
		disasm->len = 1;
		break;
	case 0x94:
		sprintf(disasm->cAsm, "XCHG AX, SP");
		disasm->len = 1;
		break;
	case 0x95:
		sprintf(disasm->cAsm, "XCHG AX, BP");
		disasm->len = 1;
		break;
	case 0x96:
		sprintf(disasm->cAsm, "XCHG AX, SI");
		disasm->len = 1;
		break;
	case 0x97:
		sprintf(disasm->cAsm, "XCHG AX, DI");
		disasm->len = 1;
		break;
	case 0x98:
		sprintf(disasm->cAsm, "CBW");
		disasm->len = 1;
		break;
	case 0x99:
		sprintf(disasm->cAsm, "CWD");
		disasm->len = 1;
		break;
	case 0x9A:
		// call far_proc
		sprintf(disasm->cAsm, "CALL %02X%02X:%02X%02X", code[4], code[3], code[2], code[1]);
		disasm->len = 5;
		break;
	case 0x9B:
		sprintf(disasm->cAsm, "WAIT");
		disasm->len = 1;
		break;
	case 0x9C:
		sprintf(disasm->cAsm, "PUSHF");
		disasm->len = 1;
		break;
	case 0x9D:
		sprintf(disasm->cAsm, "POPF");
		disasm->len = 1;
		break;
	case 0x9E:
		//stores the contents of the AH register into the lower 8 bits of the Flags 
		sprintf(disasm->cAsm, "SAHF");
		disasm->len = 1;
		break;
	case 0x9F:
		//loads the lower 8 bits of the Flags Register (status flags) into the AH register.
		sprintf(disasm->cAsm, "LAHF");
		disasm->len = 1;
		break;
	case 0xA0:
		/*
			  //d = 0(reg is source) , w = 1 (word)
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "MOV %s, %s", buffer, regName);
		break;

		*/

		sprintf(disasm->cAsm, "MOV AL, [%02X%02X]", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xA1:
		sprintf(disasm->cAsm, "MOV AX, [%02X%02X]", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xA2:
		sprintf(disasm->cAsm, "MOV [%02X%02X], AL", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xA3:
		sprintf(disasm->cAsm, "MOV [%02X%02X], AL", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xA4:
		sprintf(disasm->cAsm, "MOVSB");
		disasm->len = 1;
		break;
	case 0xA5:
		sprintf(disasm->cAsm, "MOVSW");
		disasm->len = 1;
		break;
	case 0xA6:
		sprintf(disasm->cAsm, "CMPSB");
		disasm->len = 1;
		break;
	case 0xA7:
		sprintf(disasm->cAsm, "CMPSW");
		disasm->len = 1;
		break;
	case 0xA8:
		sprintf(disasm->cAsm, "TEST AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xA9:
		sprintf(disasm->cAsm, "TEST AX, %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xAA:
		sprintf(disasm->cAsm, "STOSB");
		disasm->len = 1;
		break;
	case 0xAB:
		sprintf(disasm->cAsm, "STOSW");
		disasm->len = 1;
		break;
	case 0xAC:
		sprintf(disasm->cAsm, "LODSB");
		disasm->len = 1;
		break;
	case 0xAD:
		sprintf(disasm->cAsm, "LODSW");
		disasm->len = 1;
		break;
	case 0xAE:
		sprintf(disasm->cAsm, "SCASB");
		disasm->len = 1;
		break;
	case 0xAF:
		sprintf(disasm->cAsm, "SCASW");
		disasm->len = 1;
		break;
	case 0xB0:
		sprintf(disasm->cAsm, "MOV AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xB1:
		sprintf(disasm->cAsm, "MOV CL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xB2:
		sprintf(disasm->cAsm, "MOV DL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xB3:
		sprintf(disasm->cAsm, "MOV BL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xB4:
		sprintf(disasm->cAsm, "MOV AH, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xB5:
		sprintf(disasm->cAsm, "MOV CH, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xB6:
		sprintf(disasm->cAsm, "MOV DH, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xB7:
		sprintf(disasm->cAsm, "MOV BH, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xB8:
		sprintf(disasm->cAsm, "MOV AX, %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xB9:
		sprintf(disasm->cAsm, "MOV CX, %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xBA:
		sprintf(disasm->cAsm, "MOV DX, %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xBB:
		sprintf(disasm->cAsm, "MOV BX, %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xBC:
		sprintf(disasm->cAsm, "MOV SP, %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xBD:
		sprintf(disasm->cAsm, "MOV BP, %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xBE:
		sprintf(disasm->cAsm, "MOV SI, %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xBF:
		sprintf(disasm->cAsm, "MOV DI, %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xC2:
		sprintf(disasm->cAsm, "RET %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xC3:
		sprintf(disasm->cAsm, "RET");
		disasm->len = 1;
		break;
	case 0xC4:
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "LES %s, %s", regName, buffer);
		break;
	case 0xC5:
		regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "LDS %s, %s", regName, buffer);
		break;
	case 0xC6:
		//MOV BYTE PTR [mem], imm8
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "MOV BYTE PTR %s, %02X", buffer, code[4]);
		disasm->len += 1;
		break;
	case 0xC7:
		//MOV WORD PTR [mem], imm16
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		sprintf(disasm->cAsm, "MOV WORD PTR %s, %02X%02X", buffer, code[5], code[4]);
		disasm->len += 2;
		break;
	case 0xCA:
		sprintf(disasm->cAsm, "RETF %02X%02X", (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 3;
		break;
	case 0xCB:
		sprintf(disasm->cAsm, "RET");
		disasm->len = 1;
		break;
	case 0xCC:
		sprintf(disasm->cAsm, "INT 3");
		disasm->len = 1;
		break;
	case 0xCD:
		sprintf(disasm->cAsm, "INT %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xCE:
		sprintf(disasm->cAsm, "INTO");
		disasm->len = 1;
		break;
	case 0xCF:
		sprintf(disasm->cAsm, "IRET");
		disasm->len = 1;
		break;
	case 0xD0:
		if (modRegRm.reg == 0) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "ROL BYTE PTR %s, 1", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "ROL %s, 1", buffer);
			}

		}
		if (modRegRm.reg == 1) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "ROR BYTE PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "ROR %s, 1", buffer);
			}
		}
		if (modRegRm.reg == 2) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "RCL BYTE PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "RCL %s, 1", buffer);
			}

		}
		if (modRegRm.reg == 3) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "RCR BYTE PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "RCR %s, 1", buffer);
			}
		}
		if (modRegRm.reg == 4) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SHL BYTE PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SHL %s, 1", buffer);
			}
		}
		if (modRegRm.reg == 5) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SHR BYTE PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SHR %s, 1", buffer);
			}
		}
		if (modRegRm.reg == 7) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SAR BYTE PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SAR %s, 1", buffer);
			}
		}
		break;
	case 0xD1:
		if (modRegRm.reg == 0) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "ROL WORD PTR %s, 1", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "ROL %s, 1", buffer);
			}

		}
		if (modRegRm.reg == 1) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "ROR WORD PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "ROR %s, 1", buffer);
			}
		}
		if (modRegRm.reg == 2) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "RCL WORD PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "RCL %s, 1", buffer);
			}

		}
		if (modRegRm.reg == 3) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "RCR WORD PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "RCR %s, 1", buffer);
			}
		}
		if (modRegRm.reg == 4) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SHL WORD PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SHL %s, 1", buffer);
			}
		}
		if (modRegRm.reg == 5) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SHR WORD PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SHR %s, 1", buffer);
			}
		}
		if (modRegRm.reg == 7) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SAR WORD PTR %s, 1", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SAR %s, 1", buffer);
			}
		}
		break;
	case 0xD2:
		if (modRegRm.reg == 0) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "ROL BYTE PTR %s, CL", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "ROL %s, CL", buffer);
			}

		}
		if (modRegRm.reg == 1) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "ROR BYTE PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "ROR %s, CL", buffer);
			}
		}
		if (modRegRm.reg == 2) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "RCL BYTE PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "RCL %s, CL", buffer);
			}

		}
		if (modRegRm.reg == 3) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "RCR BYTE PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "RCR %s, CL", buffer);
			}
		}
		if (modRegRm.reg == 4) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SHL BYTE PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SHL %s, CL", buffer);
			}
		}
		if (modRegRm.reg == 5) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SHR BYTE PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SHR %s, CL", buffer);
			}
		}
		if (modRegRm.reg == 7) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SAR BYTE PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SAR %s, CL", buffer);
			}
		}
		break;
	case 0xD3:
		if (modRegRm.reg == 0) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "ROL WORD PTR %s, CL", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "ROL %s, CL", buffer);
			}

		}
		if (modRegRm.reg == 1) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "ROR WORD PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "ROR %s, CL", buffer);
			}
		}
		if (modRegRm.reg == 2) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "RCL WORD PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "RCL %s, CL", buffer);
			}

		}
		if (modRegRm.reg == 3) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "RCR WORD PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "RCR %s, CL", buffer);
			}
		}
		if (modRegRm.reg == 4) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SHL WORD PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SHL %s, CL", buffer);
			}
		}
		if (modRegRm.reg == 5) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SHR WORD PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SHR %s, CL", buffer);
			}
		}
		if (modRegRm.reg == 7) {
			decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "SAR WORD PTR %s, CL", buffer);
			}
			else {
				sprintf(disasm->cAsm, "SAR %s, CL", buffer);
			}
		}
		break;
	case 0xD4:
		if (code[1] == 0x0A) {
			sprintf(disasm->cAsm, "AAM");
			disasm->len = 2;
		}
		else {
			//if the second byte is not 0x0A, not an eligible machine code
			//print warnings and continue analyze
			sprintf(disasm->cAsm, "invalid code");
			disasm->len = 2;
		}
		break;
	case 0xD5:
		if (code[1] == 0x0A) {
			sprintf(disasm->cAsm, "AAD");
			disasm->len = 2;
		}
		else {
			//if the second byte is not 0x0A, not an eligible machine code
			//print warnings and continue analyze
			sprintf(disasm->cAsm, "invalid code");
			disasm->len = 2;
		}
		break;
	case 0xD7:
		sprintf(disasm->cAsm, "XLAT");
		disasm->len = 1;
		break;
	case 0xD8:
		//ESC opcode，TBD
		break;
	case 0xE0:
		if ((signed char)code[1] >= 0) {
			sprintf(disasm->cAsm, "LOOPNZ %02X", (signed char)code[1]);
		}
		else {
			sprintf(disasm->cAsm, "LOOPNZ -%02X", -(signed char)code[1]);
		}
		disasm->len = 2;
		break;
	case 0xE1:
		if ((signed char)code[1] >= 0) {
			sprintf(disasm->cAsm, "LOOPZ %02X", (signed char)code[1]);
		}
		else {
			sprintf(disasm->cAsm, "LOOPZ -%02X", -(signed char)code[1]);
		}
		disasm->len = 2;
		break;
	case 0xE2:
		if ((signed char)code[1] >= 0) {
			sprintf(disasm->cAsm, "LOOP %02X", (signed char)code[1]);
		}
		else {
			sprintf(disasm->cAsm, "LOOP -%02X", -(signed char)code[1]);
		}
		disasm->len = 2;
		break;
	case 0xE3:
		if ((signed char)code[1] >= 0) {
			sprintf(disasm->cAsm, "JCXZ %02X", (signed char)code[1]);
		}
		else {
			sprintf(disasm->cAsm, "JCXZ -%02X", -(signed char)code[1]);
		}
		disasm->len = 2;
		break;
	case 0xE4:
		sprintf(disasm->cAsm, "IN AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xE5:
		sprintf(disasm->cAsm, "IN AX, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xE6:
		sprintf(disasm->cAsm, "OUT AL, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xE7:
		sprintf(disasm->cAsm, "OUT AX, %02X", (unsigned char)code[1]);
		disasm->len = 2;
		break;
	case 0xE8:
		//call near proc
		if (((signed short)code[1] | (signed short)code[2] << 8) >= 0) {
			sprintf(disasm->cAsm, " CALL %04X", (signed short)code[1] | (signed short)code[2] << 8);
		}
		else {
			sprintf(disasm->cAsm, " CALL -%04X", -(signed short)code[1] | (signed short)code[2] << 8);
		}
		disasm->len = 3;
		break;
	case 0xE9:
		// jmp near label
		if (((signed short)code[1] | (signed short)code[2] << 8) >= 0) {
			sprintf(disasm->cAsm, "JMP %04X", (signed short)code[1] | (signed short)code[2] << 8);
		}
		else {
			sprintf(disasm->cAsm, "JMP -%04X", -(signed short)code[1] | (signed short)code[2] << 8);
		}
		disasm->len = 3;
		break;
	case 0xEA:
		//JMP far-label
		sprintf(disasm->cAsm, "JMP %02X%02X:%02X%02X", (unsigned char)code[4], (unsigned char)code[3], (unsigned char)code[2], (unsigned char)code[1]);
		disasm->len = 5;
	case 0xEB:
		//jmp short label
		if ((signed char)code[1] >= 0) {
			sprintf(disasm->cAsm, "JMP %02X", (signed char)code[1]);
		}
		else {
			sprintf(disasm->cAsm, "JMP -%02X", -(signed char)code[1]);
		}
		disasm->len = 2;
		break;
	case 0xEC:
		sprintf(disasm->cAsm, "IN AL, DX");
		disasm->len = 1;
		break;
	case 0xED:
		sprintf(disasm->cAsm, "IN AX, DX");
		disasm->len = 1;
		break;
	case 0xEE:
		sprintf(disasm->cAsm, "OUT AL, DX");
		disasm->len = 1;
		break;
	case 0xEF:
		sprintf(disasm->cAsm, "OUT AX, DX");
		disasm->len = 1;
		break;
	case 0xF0:
		sprintf(disasm->cAsm, "LOCK");
		disasm->len = 1;
		break;
	case 0xF2: 
		sprintf(disasm->cAsm, "REPNZ");
		disasm->len = 1;
		break;
	case 0xF3:
		sprintf(disasm->cAsm, "REP");
		disasm->len = 1;
		break;
	case 0xF4:
		sprintf(disasm->cAsm, "HLT");
		disasm->len = 1;
		break;
	case 0xF5:
		sprintf(disasm->cAsm, "CMC");
		disasm->len = 1;
		break;
	case 0xF6:
			//regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		if (modRegRm.reg == 0) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "TEST BYTE PTR %s, %02X", buffer, (unsigned char)code[disasm->len]);
			}
			else
			{
				sprintf(disasm->cAsm, "TEST %s, %02X", buffer, (unsigned char)code[disasm->len]);
			}
			disasm->len += 1;
		}
		if (modRegRm.reg == 2) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "NOT BYTE PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "NOT %s", buffer);
			}
		}
		if (modRegRm.reg == 3) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "NEG BYTE PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "NEG %s", buffer);
			}
		
		}
		if (modRegRm.reg == 4) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "MUL BYTE PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "MUL %s", buffer);
			}
		}
		if (modRegRm.reg == 5) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "IMUL BYTE PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "IMUL %s", buffer);
			}
		
		}
		if (modRegRm.reg == 6) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "DIV BYTE PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "DIV %s", buffer);
			}
		}
		if (modRegRm.reg == 7) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, " IDIV BYTE PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "IDIV %s", buffer);
			}

		}
		/*else if (modRegRm.reg == 2) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "TEST BYTE PTR %s, %02X", buffer, (unsigned char)code[4]);
			}
			else
			{
				sprintf(disasm->cAsm, "TEST %s, %02X", buffer, (unsigned char)code[4]);
			}
		}*/
		break;
	case 0xF7:
		//regName = Register16[modRegRm.reg];
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		if (modRegRm.reg == 0) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "TEST WORD PTR %s, %02X%02X", buffer, (unsigned char)code[disasm->len + 1], (unsigned char)code[disasm->len]);
			}
			else
			{
				sprintf(disasm->cAsm, "TEST %s, %02X%02X", buffer, (unsigned char)code[disasm->len + 1], (unsigned char)code[disasm->len]);
			}
			disasm->len += 2;
		}
		if (modRegRm.reg == 2) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "NOT WORD PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "NOT %s", buffer);
			}
		}
		if (modRegRm.reg == 3) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "NEG WORD PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "NEG %s", buffer);
			}

		}
		if (modRegRm.reg == 4) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "MUL WORD PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "MUL %s", buffer);
			}
		}
		if (modRegRm.reg == 5) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "IMUL WORD PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "IMUL %s", buffer);
			}

		}
		if (modRegRm.reg == 6) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "DIV WORD PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "DIV %s", buffer);
			}
		}
		if (modRegRm.reg == 7) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, " IDIV WORD PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "IDIV %s", buffer);
			}

		}
		break;
	case 0xF8:
		sprintf(disasm->cAsm, "CLC");
		disasm->len = 1;
		break;
	case 0xF9:
		sprintf(disasm->cAsm, "STC");
		disasm->len = 1;
		break;
	case 0xFA:
		sprintf(disasm->cAsm, "CLI");
		disasm->len = 1;
		break;
	case 0xFB:
		sprintf(disasm->cAsm, "STI");
		disasm->len = 1;
		break;
	case 0xFC:
		sprintf(disasm->cAsm, "CLD");
		disasm->len = 1;
		break;
	case 0xFD:
		sprintf(disasm->cAsm, "STD");
		disasm->len = 1;
		break;
	case 0xFE: 
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		if (modRegRm.reg == 0) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "INC BYTE PTR %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "INC %s", buffer);
			}
		}
		if (modRegRm.reg == 1) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "DEC BYTE PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "DEC %s", buffer);
			}
		}
		break;
	case 0xFF:
		decodeModRm(buffer, modRegRm.mod, modRegRm.rm, code, disasm);
		if (modRegRm.reg == 0) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "INC WORD PTR %s", buffer); 
			}
			else
			{
				sprintf(disasm->cAsm, "INC %s", buffer);
			}
		}
		if (modRegRm.reg == 1) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "DEC WORD PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "DEC %s", buffer);
			}
		}
		if (modRegRm.reg == 2) {
			//call near
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "CALL WORD PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "CALL %s", buffer);
			}

		}
		if (modRegRm.reg == 3) {
			//call far
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "CALL FAR PTR  %s", buffer);
			}
		}
		if (modRegRm.reg == 4) {
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "JMP WORD PTR  %s", buffer);
			}
			else
			{
				sprintf(disasm->cAsm, "JMP %s", buffer);
			}

		}
		if (modRegRm.reg == 5) {
			//jmp far
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "JMP FAR PTR  %s", buffer);
			}
		}
		if (modRegRm.reg == 6)
		{
			if (modRegRm.mod != 0x03) {
				sprintf(disasm->cAsm, "PUSH WORD PTR  %s", buffer);
			}
		}
		break;
	default:
		break;
	}
	return disasm->len;
}

int main()
{
#if 0
	FILE* file = fopen("test.txt", "rb");
	if (file == NULL) {
		perror("Error opening file");
		return 1;
	}

	char buffer[6];
	int bytesRead;
	int totalProcessed = 0;

	while (1) {
		bytesRead = fread(buffer, 1, 6, file);
		if (bytesRead <= 0) {
			if (feof(file)) {
				break; // 文件已经读取完毕
			}
			else {
				perror("Error reading file");
				fclose(file);
				return 1; // 读取文件时发生错误
			}
		}

		stDisasm disasm;
		int processed = Disasm(buffer, bytesRead, &disasm);
		if (processed < 0) {
			printf("Error occurred during disassembly.\n");
			fclose(file);
			return 1; //  An error occurred while parsing
		}
		totalProcessed += processed;

		// Adjust the file pointer to skip bytes that have already been processed
		fseek(file, processed - bytesRead, SEEK_CUR);
		printf(" %s:\r\n", disasm.cAsm);
	}

	printf("Total bytes processed: %d\n", totalProcessed);

	fclose(file);
#endif // 0

	//test code
	char testCode[] = { 0x00, 0xC8,  0x01, 0xC8, 0x02, 0x00, 0x03, 0x00, 0x04, 0x05, 0x05, 0x12, 0x80, 0x80, 0xC3, 0x05, 0x81, 0xC3, 0x34, 0x12, 0x82, 0xC0, 0x12,0x83, 0xC3, 0xFC,
									0x06, 0x07, 0x08, 0xD8, 0x09, 0xD8, 0x0A, 0x07, 0x0B, 0x0A, 0x0D, 0x78, 0x56, 0x0C, 0x12, 0x0E, 0x10, 0xC8, 0x11, 0xC8, 0x12, 0x00, 0x13, 0x05, 0x14, 0x05, 0x15, 0x34, 0x12,
									0x16, 0x17, 0x18, 0xC8, 0x19, 0xC8, 0x1A, 0x00, 0x1B, 0x04, 0x1C, 0x34, 0x1D, 0x78, 0x56, 0x1E, 0x1F,
									0x20, 0xC9, 0x21, 0xC8, 0x22, 0x01, 0x23, 0x00, 0x24, 0x06, 0x25, 0x12, 0x45, 0x26, 0x27,
									0x28, 0xC8, 0x29, 0xC8, 0x2A, 0x02, 0x2B, 0x03, 0x2C, 0x09, 0x2D, 0x12, 0x35, 0x2E, 0x2F,
									0x30, 0xC8, 0x31, 0xD1, 0x32, 0x04, 0x33, 0x05, 0x34, 0x1A, 0x35, 0x12, 0xA5, 0x36, 0x37,
									0x38, 0xCA, 0x39, 0xD0 , 0x3A, 0x00, 0x3B, 0x02, 0x3C, 0x1A, 0x3D, 0x1A, 0xB5, 0x3E, 0x3F,
									0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
									0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
									0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
									0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
									0x70, 0xC1, 0x71, 0x05, 0x72, 0x06, 0x73, 0x10, 0x74, 0x1A, 0x75, 0x01, 0x76, 0x02, 0x77, 0x03, 0x78, 0x04, 0x79, 0x05, 0x7A, 0x80, 0x7B, 0x1A, 0x7C, 0xC1, 0x7D, 0xC2, 0x7E, 0xC3, 0x7F, 0xC4,
									0x80, 0xCB, 0x05, 0x80, 0xD3, 0x06, 0x80, 0xD9, 0x07,0x80, 0xE2, 0xA1, 0x80, 0xF5, 0x05, 0x80, 0xFE, 0xC1, 0x80, 0xE9, 0x09,
									0x81, 0xCB, 0x24, 0x01, 0x81, 0xD3, 0x34, 0x02, 0x81, 0xD9, 0x12, 0x05, 0x81, 0xE2, 0x15, 0x02, 0x81, 0xE9, 0x02, 0x01, 0x81, 0xF2, 0x11, 0x01, 0x81, 0xFB, 0x11, 0x11,
								   0x82, 0xD3, 0x06, 0x82, 0xD9, 0x07,  0x82, 0xFE, 0x09,
								   0x83, 0xC3, 0xFC,  0x83, 0xD3, 0x34,0x83, 0xD9, 0x12, 0x83, 0xE9, 0x02, 0x83, 0xFB, 0x11,
									0x84, 0xC0, 0x85, 0xD1, 0x86, 0xC3, 0x87, 0xC3, 0x8C, 0xC0, 0x8D, 0x44, 0x10, 0x8E, 0xC0, 0x8F, 0x44, 0x09, 0x8F, 0x07,
									0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
									0x9A, 0x34, 0x12, 0x78, 0x56,
									0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
									0xA0, 0x08, 0x01, 0xA1, 0x02, 0x01, 0xA2, 0x02, 0x01, 0xA3, 0x01, 0x02,
									0xA4, 0xA5, 0xA6, 0xA7,
									0xA8, 0x10, 0xA9, 0x01, 0x02, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
									0xB0, 0x01, 0xB1, 0x01, 0xB2, 0x03, 0xB3, 0x01, 0xB4, 0x01, 0xB5, 0x02, 0xB6, 0x03, 0xB7, 0x04,
									0xB8, 0x01, 0x02, 0xB9, 0x01, 0x02, 0xBA, 0x02, 0x03, 0xBB, 0x01, 0x02, 0xBC, 0x01, 0x02, 0xBD, 0x02, 0x03, 0xBE, 0x02, 0x03, 0xBF, 0x01, 0x02,
									0xC2, 0x01, 0x02, 0xC3, 0xC4, 0x0E, 0x78, 0x56, 0xC5, 0x0E, 0x78, 0x56,
									0xC6,  0x0E, 0x78, 0x56, 0x09, 0xC7, 0x06, 0x34, 0x12,  0x09, 0x78,
									0xCA, 0x04, 0x00, 0xCB, 0xCC, 0xCD, 0x02, 0xCE, 0xCF,
									0xD0, 0xC0, 0xD0, 0x04, 0xD0, 0x06, 0x34, 0x12, 0xD0, 0xD1, 0xD0, 0xC8, 0xD0, 0xD8, 0xD0, 0xE4, 0xD0, 0xE8,0xD0, 0x3E, 0x78, 0x56, 0xD0, 0xF8,
									0xD1, 0xC0, 0xD1, 0x04, 0xD1, 0x06, 0x34, 0x12, 0xD1, 0xD1, 0xD1, 0xC8, 0xD1, 0xD8, 0xD1, 0xE4, 0xD1, 0xE8,0xD1, 0x3E, 0x78, 0x56, 0xD1, 0xF8,
									0xD2, 0xC0, 0xD2, 0x04, 0xD2, 0x06, 0x34, 0x12, 0xD2, 0xD1, 0xD2, 0xC8, 0xD2, 0xD8, 0xD2, 0xE4, 0xD2, 0xE8,0xD2, 0x3E, 0x78, 0x56, 0xD2, 0xF8,
									0xD3, 0xC0, 0xD3, 0x04, 0xD3, 0x06, 0x34, 0x12, 0xD3, 0xD1, 0xD3, 0xC8, 0xD3, 0xD8, 0xD3, 0xE4, 0xD3, 0xE8,0xD3, 0x3E, 0x78, 0x56, 0xD3, 0xF8, 0xD3, 0x08,
									0xD4, 0x02, 0xD4, 0x0A, 0xD5, 0x01, 0xD5, 0x0A,
									0xE0, 0xFB, 0xE1, 0xFB, 0xE2, 0xF2, 0xE3, 0x02,
									0xE4, 0x23, 0xE5, 0xB2, 0xE6, 0xD4, 0xE7, 0xA0,
									0xE8, 0xE3, 0x02,0xE8, 0x01, 0x02, 0xE9, 0x12, 0x34, 
									0xEA, 0x78, 0x56, 0x34, 0x12, 0xEB, 0xE2,
									0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF2, 0xF3, 0xF4, 0xF5,
									0xF6, 0x06, 0x0F, 0x01, 0x02, 0xF6, 0xC0, 0x08, 0xF6, 0x00, 0x08,
									0xF6, 0xD0, 0xF6, 0x12, 0xF6, 0xD8, 0xF6, 0xE0, 0xF6, 0xEA, 0xF6, 0xF3, 0xF6, 0xFB, 
									0xF7, 0xC0, 0x00, 0xFF, 0xF7, 0xD0, 0xF7, 0xD8, 0xF7, 0xE3, 0xF7, 0xEB, 0xF7, 0xF3, 0xF7, 0xFB,
									0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 
									0xFE, 0xC8, 0xFE, 0x0F, 
									0xFF, 0xC0, 0xFF, 0xC8, 0xFF, 0x10, 0xFF, 0x18, 0xFF, 0x20, 0xFF, 0x28, 
									0xB8, 0x01, 0x00, //MOV     AX,0001
									0xBB, 0x02, 0x00, //MOV     BX,0002
									0x01, 0xD8,       //ADD     AX,BX
									0xC3,			//ret
	
	
	};
	int curPos = 0; //current position
	while (curPos < sizeof(testCode))
	{
		stDisasm disasm;
		memset(&disasm, 0, sizeof(disasm));

		// call disasm
		int result = Disasm(&testCode[curPos], sizeof(testCode) - curPos, &disasm);

		if (result > 0) {
			printf("Disassembled code: %s\n", disasm.cAsm);
			curPos += result;
		}
		else {
			printf("Error in disassembly.\n");
			break;
		}

	}


	return 0;
}





