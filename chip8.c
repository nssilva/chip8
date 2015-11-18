#include "chip8.h"

// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

int do_log(cpu *cp)
{
	FILE *out;
	
	out = fopen("out.txt", "a+");
	
	if(out == 0)
	{
		printf("cant't open file for writing");
	}

	fprintf(out, "Unrecognized opcode 0x%04x\n", cp->opcode);
	fclose(out);
	
	return 0;
}

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
	memset(cp->gfx, 0x00, sizeof(*cp->gfx) * 32 * 64);
}

void do_call(cpu *cp)
{
	printf("call\n");
	cp->stack[STACKMASK & ++cp->sp] = cp->pc;
	setpc(cp, cp->opcode & 0xfff);
}

void do_jp(cpu *cp)
{
	printf("jp\n");
	setpc(cp, cp->opcode & 0xfff);
}

void do_ret(cpu *cp)
{
	setpc(cp, cp->stack[STACKMASK & cp->sp--]);
}

void do_se(cpu *cp)
{
	if(cp->v[cp->opcode & 0x0f00 >> 8] == (cp->opcode & 0x00ff))
	{
		cp->pc +=4;
	}
	else
	{
		cp->pc +=2;
	}
}

void do_sne(cpu *cp)
{
	if(cp->v[cp->opcode & 0x0f00 >> 8] != (cp->opcode & 0x00ff))
	{
		cp->pc +=4; //skips next instruction witch is usually cp->pc +=2;
	}
	else
	{
		cp->pc +=2;
	}
}

void set_vx_kk(cpu *cp)
{
	cp->v[(cp->opcode & 0xF00) >> 8] = cp->opcode & 0x00FF;
	cp->pc +=2;
}

void do_catchall(cpu *cp)
{
	printf("Unrecognized opcode 0x%04x\n", cp->opcode);
	do_log(cp);	
}

opcode opcodes[] = {
{0xffff, 0x00e0, do_cls},
{0xf000, 0x1000, do_jp},
{0xf000, 0x2000, do_call},
{0xf000, 0x3000, do_se},
{0xf000, 0x4000, do_sne},
{0xf000, 0x6000, set_vx_kk},
{0xffff, 0x00ee, do_ret},
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
	
	memcpy(&cp->memory[FONT_BASE], fontset, FONT_SIZE);
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
