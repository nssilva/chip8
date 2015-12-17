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
	setpc(cp, cp->opcode & 0x0fff);
}

void do_ret(cpu *cp)
{
	setpc(cp, cp->stack[STACKMASK & cp->sp--]);
}

void do_skp_vx_kk(cpu *cp)
{
	if(cp->v[cp->opcode & 0x0f00 >> 8] == (cp->opcode & 0x00ff))
	{
		cp->pc +=2;//skips next instruction because the singlestep do the first cp->pc +=2;
	}
}

void do_skp_vx_not_kk(cpu *cp)
{
	if(cp->v[cp->opcode & 0x0f00 >> 8] != (cp->opcode & 0x00ff))
	{
		cp->pc +=2; //skips next instruction because the singlestep do the first cp->pc +=2;
	}
}

void do_skp_vx_vy(cpu *cp)
{
	if(cp->v[(cp->opcode & 0x0F00) >> 8] == ((cp->opcode & 0x0F00) >> 4))
	{
		cp->pc +=2;	
	}
}

#define IMM8 (cp->opcode & 0x00ff)
#define VX   cp->v[(cp->opcode & 0x0f00) >> 8]
#define VY   cp->v[(cp->opcode & 0x00f0) >> 4]
#define VF   cp->v[0xf] 

void do_set_vx_kk(cpu *cp)
{
	cp->v[(cp->opcode & 0x0f00) >> 8] = (cp->opcode & 0x00FF);
}

void do_set_vx_kk_add(cpu *cp)
{
	VX += IMM8;
	//cp->v[(cp->opcode & 0x0f00) >> 8] += (cp->opcode & 0x00ff);
}

void do_ld_vx_vy(cpu *cp)
{
	VX = VY;
}

void do_or_vx_vy(cpu *cp)
{
	VX |= VY;
}

void do_and_vx_vy(cpu *cp)
{
	VX &= VY;
}

void do_xor_vx_vy(cpu *cp)
{
	VX ^= VY;
}

void do_skp_vx_not_vy(cpu *cp)
{
	if(cp->v[(cp->opcode & 0x0f00) >> 8] != cp->v[(cp->opcode & 0x00f0)])
	{
		cp->pc +=2;
	}
}

void do_set_i_nnn(cpu *cp)
{
	cp->i = (cp->opcode & 0x0fff);
}

void do_jp_v0(cpu *cp)
{
	printf("Another jp\n");
	setpc(cp, (cp->opcode & 0xfff) + cp->v[0]);
}

void do_set_vx_rnd_kk(cpu *cp)
{
	WORD rnd = rand();
	
	rnd &= (cp->opcode & 0x00ff);
	
	cp->v[(cp->opcode & 0x0f00) >> 8] = rnd;
}

void do_add_vx_vy(cpu *cp) 
{
	VF = (VX + VY) > 0xff; 
	VX += VY;
	printf("I HAVE BEEN CALLED YUPII");
}

void do_sub_vx_vy(cpu *cp)
{
	VF = (VX >= VY);
	VX -= VY;
}

void do_shr_vx(cpu *cp)
{
	VF = (VX & 0x0001); /*shr = shift right >> */
	
	VX >>= 1; /*this is the same as divide by two*/
}

void do_subn_vx_vy(cpu *cp)
{
	VX = VY - VX;
	VF = (VY > VX);
	VX -= VY;
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
{0xf000, 0x3000, do_skp_vx_kk},
{0xf000, 0x4000, do_skp_vx_not_kk},
{0xf000, 0x5000, do_skp_vx_vy},
{0xf000, 0x6000, do_set_vx_kk},
{0xf000, 0x7000, do_set_vx_kk_add},
{0xf00f, 0x8000, do_ld_vx_vy},
{0xf00f, 0x8001, do_or_vx_vy},
{0xf00f, 0x8002, do_and_vx_vy},
{0xf00f, 0x8003, do_xor_vx_vy},
{0xf00f, 0x8004, do_add_vx_vy},
{0xf00f, 0x8005, do_sub_vx_vy},
{0xf00f, 0x8006, do_shr_vx},
{0xf00f, 0x8007, do_subn_vx_vy},
{0xf000, 0x9000, do_skp_vx_not_vy},
{0xf000, 0xa000, do_set_i_nnn},
{0xffff, 0x00ee, do_ret},
{0xf000, 0xb000, do_jp_v0},
{0xf000, 0xc000, do_set_vx_rnd_kk},
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
