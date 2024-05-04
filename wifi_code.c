#include "base.h"

#define WIFI_RAM_SIZE 512*1024

typedef union
{
	struct
	{
		byte frac;
		int whole : 24;
	} fixed;
	int val;
} fixed8;

void nothing()
{
	dword lol;
	deref(0x1000) = *(((byte*)&lol)+4);
}


void main();

typedef struct
{
	char screen[40*16];
} shared_t;

__attribute__( ( used, section( ".shared" ) ) ) volatile shared_t shared;


__attribute__( ( used, section( ".int.table" ) ) ) void* interrupt_table[64] = {
	0,       &main,   &nothing,&nothing,&nothing,&nothing,&nothing,&nothing,
	&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,
	&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,
	&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,
	&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,
	&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,
	&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,
	&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing,&nothing
};

static dword FAST_sqrt(fixed8 fixeda8)
{
	dword a = fixeda8.val;
	dword sqrtA = 0;
	dword sqrtB = 0;
	dword ret = 0;
	sqrtA |= a&(1<<0); a >>= 1; sqrtB |= a&(1<<0);
	sqrtA |= a&(1<<1); a >>= 1; sqrtB |= a&(1<<1);
	sqrtA |= a&(1<<2); a >>= 1; sqrtB |= a&(1<<2);
	sqrtA |= a&(1<<3); a >>= 1; sqrtB |= a&(1<<3);
	sqrtA |= a&(1<<4); a >>= 1; sqrtB |= a&(1<<4); 
	sqrtA |= a&(1<<5); a >>= 1; sqrtB |= a&(1<<5); 
	sqrtA |= a&(1<<6); a >>= 1; sqrtB |= a&(1<<6); 
	sqrtA |= a&(1<<7); a >>= 1; sqrtB |= a&(1<<7); 
	sqrtA |= a&(1<<8); a >>= 1; sqrtB |= a&(1<<8); 
	sqrtA |= a&(1<<9); a >>= 1; sqrtB |= a&(1<<9); 
	sqrtA |= a&(1<<10); a >>= 1; sqrtB |= a&(1<<10);
	sqrtA |= a&(1<<11); a >>= 1; sqrtB |= a&(1<<11);
	sqrtA |= a&(1<<12); a >>= 1; sqrtB |= a&(1<<12);
	sqrtA |= a&(1<<13); a >>= 1; sqrtB |= a&(1<<13);
	sqrtA |= a&(1<<14); a >>= 1; sqrtB |= a&(1<<14);
	sqrtA |= a&(1<<15); a >>= 1; sqrtB |= a&(1<<15);
	
	if(sqrtA > sqrtB)
	{
		ret = sqrtA;
	}
	else
	{
		ret = sqrtB;
	}
	
	ret = (ret>>1) + ((fixeda8.val<<7)/ret);
	ret = (ret>>1) + ((fixeda8.val<<7)/ret);
	ret = (ret>>1) + ((fixeda8.val<<7)/ret);
	ret = (ret>>1) + ((fixeda8.val<<7)/ret);
	
	return ret;
}

static dword SDF_SPHERE(fixed8 x, fixed8 y, fixed8 z, fixed8 sx, fixed8 sy, fixed8 sz, fixed8 ss)
{
	x.val -= sx.val;
	y.val -= sy.val;
	z.val -= sz.val;
	fixed8 dstsqr;
	dstsqr.val = ((x.val*x.val)>>8)+((y.val*y.val)>>8)+((z.val*z.val)>>8);
	return FAST_sqrt(dstsqr)-ss.val;
}

static int ABS(int a)
{
	if(a < 0)
	{
		return -a;
	}
	return a;
}

static int MAX0(int a)
{
	if(a < 0)
	{
		return 0;
	}
	return a;
}

static int MIN(int a, int b)
{
	if(a < b)
	{
		return a;
	}
	return b;
}

static dword SDF_CUBE(fixed8 x, fixed8 y, fixed8 z, fixed8 sx, fixed8 sy, fixed8 sz, fixed8 ss)
{
	x.val -= sx.val;
	y.val -= sy.val;
	z.val -= sz.val;
	x.val = MAX0(ABS(x.val)-ss.val);
	y.val = MAX0(ABS(y.val)-ss.val);
	z.val = MAX0(ABS(z.val)-ss.val);
	fixed8 dstsqr;
	dstsqr.val = ((x.val*x.val)>>8)+((y.val*y.val)>>8)+((z.val*z.val)>>8);
	return FAST_sqrt(dstsqr);
}


static dword LENGTH(fixed8 x, fixed8 y, fixed8 z)
{
	fixed8 lensqr;
	lensqr.val = ((x.val*x.val)>>8)+((y.val*y.val)>>8)+((z.val*z.val)>>8);
	return FAST_sqrt(lensqr);
}


__attribute__( (naked, used, section( ".boot.entry" ) ) ) void main()
{
	
	__asm__("ldr r0,=0x00072000\n"
			"msr msp,r0");
	//fixed8 a;
	//a.fixed.whole = 2700;
	//a.fixed.frac = 0;
	//*(dword*)(&shared.screen[16]) = FAST_sqrt(a);
	//deref(0x1008) = 0x20202020;
	
	fixed8 fx;
	fixed8 fy;
	fixed8 fz;
	
	fixed8 dx;
	fixed8 dy;
	fixed8 dz;
	
	fixed8 sx;
	fixed8 sy;
	fixed8 sz;
	fixed8 ss;
	
	fixed8 sx2 = {.val = 5<<8};
	fixed8 sy2 = {.val = 2<<8};
	fixed8 sz2 = {.val = 2<<8};
	fixed8 ss2 = {.val = 2<<8};
	
	sx.val = 0;
	sy.val = 0;
	sz.val = 1;
	ss.val = 1<<8;
	int ox = 0;
	int oy = 0;
	while(1)
	{
		for(dword y = 0; y < 16; y++)
		{
			dword yi = y*40;
			for(dword x = 0; x < 40; x++)
			{
				dy.val = y << 5;
				dy.val -= 0x100;
				fx.val = -0x400+ABS(ox-0x800)*2;
				fy.val = -0x400+ABS(oy-0x800)*2;
				fz.val = -(2<<8);
				dz.val = 0x80;
				dword i = x+yi+0x1000;
				dx.val = x << 4;
				dx.val -= 0x100;
				fixed8 len;
				len.val = LENGTH(dx,dy,dz);
				dx.val <<= 8;
				dx.val /= len.val;
				dy.val <<= 8;
				dy.val /= len.val;
				dz.val <<= 8;
				dz.val /= len.val;
				//*(byte*)i = dx.val>>3;
				*(byte*)i = ' ';
				for(dword j = 0; j < 12; j++)
				{
					len.val = SDF_CUBE(fx,fy,fz,sx,sy,sz,ss);
					len.val = MIN(len.val,SDF_CUBE(fx,fy,fz,sx2,sy2,sz2,ss2));
					if(len.val < 0x40)
					{
						*(byte*)i = '\x00';
						break;
					}
					//*(byte*)i = dx.val;
					fx.val += (dx.val*len.val)>>8;
					fy.val += (dy.val*len.val)>>8;
					fz.val += (dz.val*len.val)>>8;
				}
			}
		}
		ox = (ox+0x20)&0xfff;
		oy = (oy+0x23)&0xfff;
	}
}