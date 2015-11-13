#include "chip8.h"

BYTE r8(cpu *cp, int addr)
{
	return cp->memory[addr & MEMMASK];
}

WORD r16(cpu *cp, int addr)
{
	return (r8(cp, addr)<<8) | r8(cp, addr+1);
}

void setpc(cpu *cp, int pc)
{
	cp->pc = pc & MEMMASK & ~1;
}

WORD fetchop(cpu *cp)
{
	WORD op = r16(cp, cp->pc);
	setpc(cp, cp->pc+2);
	return op;
}

void do_cls(cpu *cp)
{
	printf("cls\n");
}

void do_call(cpu *cp)
{
	printf("call\n");
}

void do_jp(cpu *cp)
{
	printf("jp\n");
	setpc(cp, cp->opcode & 0xfff);
}

void do_catchall(cpu *cp)
{
	printf("Unrecognized opcode 0x%04x\n", cp->opcode);
}

opcode opcodes[] = {
{0xffff, 0x00e0, do_cls},
{0xf000, 0x2000, do_call},
{0xf000, 0xb000, do_jp},
{0x0000, 0x0000, do_catchall} // MAKE THIS LAST!!!!
};


void singlestep(cpu *cp)
{
	cp->opcode = fetchop(cp);
	int count = sizeof(opcodes) / sizeof(opcodes[0]);
	int i;
	for(i=0;i<count;++i)
	{
		opcode *o = opcodes+i;
		if((cp->opcode & o->mask) == o->comp)
		{
			o->func(cp);
			break;
		}
	}
}


void init(cpu *cp)
{
	memset(cp, 0, sizeof(*cp));
	cp->pc = 0x200;
}

int loadrom(cpu *cp, char *path)
{
	FILE *rom;
	
	rom = fopen(path, "rb");
	
	if(!rom) {
		printf("Can not read file");
		return -1;
	}
		
	int want = sizeof(cp->memory);
	int len = fread(cp->memory, 1, want, rom);
	fclose(rom);

	if(len<0)
	{
		printf("Error (%d) on read of %s: %m\n", len, path);
		return -2;
	}

	if(len!=want) // can probably ignore
	{
	}

	return 0;
}

int main(int argc, char *argv[])
{
	cpu *cp = malloc(sizeof(*cp));
	init(cp);
	
	if(argc < 2) {
		printf("You must specify the game path");
		return -1;
	}

	loadrom(cp, argv[1]);		

	for(;;)
	{
		singlestep(cp);
	}
	return 0;
}
