#include "base.h"
#include "usb.h"
#include "wifi_code_compiled.h"

#define XOSC_BASE 0x40024000UL
#define XOSC_CTRL (XOSC_BASE + 0x00)
#define XOSC_STATUS (XOSC_BASE + 0x04)
#define XOSC_STARTUP (XOSC_BASE + 0x0C)

#define CLOCKS_BASE 0x40008000UL
#define CLK_REF_CTRL (CLOCKS_BASE + 0x30)
#define CLK_REF_DIV (CLOCKS_BASE + 0x34)
#define CLK_SYS_CTRL (CLOCKS_BASE + 0x3C)
#define CLK_PERI_CTRL (CLOCKS_BASE + 0x48)
#define CLK_SYS_RESUS_CTRL_RW       (CLOCKS_BASE+0x78+0x0000)

#define RESETS_BASE 0x4000C000UL
#define RESETS_RESET (RESETS_BASE + 0x00)
#define RESETS_RESET_DONE (RESETS_BASE + 0x08)

#define CORTEX_BASE 0xe0000000UL
#define CORTEX_SYST_CSR (CORTEX_BASE + 0xe010)
#define CORTEX_SYST_RVR (CORTEX_BASE + 0xe014)
#define CORTEX_SYST_CVR (CORTEX_BASE + 0xe018)

// IO Bank
#define IO_BANK0_BASE 0x40014000UL
#define IO_BANK0_GPIO(x) (IO_BANK0_BASE + x*8 + 0x04)
#define IO_BANK0_GPIO00_CTRL (IO_BANK0_BASE + 0x04)
#define IO_BANK0_GPIO14_CTRL (IO_BANK0_BASE + 0x74)
#define IO_BANK0_GPIO15_CTRL (IO_BANK0_BASE + 0x7c)

// SIO
#define SIO_BASE 0xD0000000UL
#define SIO_GPIO_IN (SIO_BASE + 0x04)
#define SIO_GPIO_OUT (SIO_BASE + 0x10)
#define SIO_GPIO_OUT_SET (SIO_BASE + 0x14)
#define SIO_GPIO_OUT_CLR (SIO_BASE + 0x18)
#define SIO_GPIO_OUT_XOR (SIO_BASE + 0x1c)
#define SIO_GPIO_OE (SIO_BASE + 0x20)
#define SIO_GPIO_OE_SET (SIO_BASE + 0x24)
#define SIO_GPIO_OE_CLR (SIO_BASE + 0x28)


#define PLL_SYS_BASE                0x40028000UL
#define PLL_SYS_CS                  (PLL_SYS_BASE + 0x00)
#define PLL_SYS_POW                 (PLL_SYS_BASE + 0x04)
#define PLL_SYS_FBDIV               (PLL_SYS_BASE + 0x08)
#define PLL_SYS_PLL                 (PLL_SYS_BASE + 0x0c)

static void Display();
static void Firstpart();
static void Secondpart();
static void Thirdpart();
static void Fourthpart();
static void ShortSync();
static void LongSync();

extern const byte _binary_image_raw_start[];

//void int_usb()
//{
//	deref(SIO_GPIO_OUT_XOR) = 1 << 0;  // XOR the LED pin
	//usb_device->ep0_buf_a[0] = '\x42';
	//usb_device->ep0_buf_a[1] = '\x69';
	//usb_device->ep_buf_ctrl[0].in = 2 + 0x400u + 0x8000 + 0x2000;
	//deref(0x00100000) = 0xa;
	/*
	dword status = usb_regs->ints;
	
	if(status & 0x00010000)
	{
		volatile struct usb_setup_packet *pkt = (volatile struct usb_setup_packet*)&usb_device->setup_packet;
		byte req_direction = pkt->bmRequestType;
		byte req = pkt->bRequest;
		
	}
	*/
//}

__attribute__( ( used, section( ".int.table" ) ) ) void* interrupt_table[64] = {
	0x20042000u,0x20000200u,0,0,0,0,0,0,
	0,          0,          0,0,0,0,0,&LongSync,
	0,          0,          0,0,0,0,0,0,
	0,          0,          0,0,0,0,0,0,
	0,          0,          0,0,0,0,0,0,
	0,          0,          0,0,0,0,0,0,
	0,          0,          0,0,0,0,0,0,
	0,          0,          0,0,0,0,0,0
};

static void clock_init ( void )
{
    deref(XOSC_CTRL)=0xAA0;      //1 - 15MHZ
    deref(XOSC_STARTUP)=0xc4;      //straight from the datasheet
    deref(XOSC_CTRL + 0x2000)=0xFAB000;  //enable
    while(1)
    {
        if((deref(XOSC_STATUS)&0x80000000)!=0) break;
    }
    deref(CLK_REF_CTRL)=2; //XOSC
    deref(CLK_SYS_CTRL)=0; //reset/clk_ref
	deref( CLK_REF_DIV ) = ( 1 << 8 );                 // CLK REF Divisor = 1
    deref( CLK_PERI_CTRL ) = ( 1 << 11 ) | ( 4 << 5 ); // CLK PERI Enable & AUX SRC = xosc_clksrc
	
	
	*(volatile dword*)(RESETS_RESET) &= ~(1<<12);
	while( (deref(RESETS_RESET_DONE) & (1<<12)) == 0 )
	{
		
	}
	
	deref(PLL_SYS_FBDIV) = 50;
	deref(PLL_SYS_PLL) = (6<<16)+(1<<12);
	deref(PLL_SYS_POW | 0x3000) = 0b100001;
	while ( deref( PLL_SYS_CS ) & ( 1 << 31 ) == 0 );
	deref( PLL_SYS_POW | 0x3000 ) = ( 1 << 3 ) ;                    // Power Up PLL Post Div
    deref( CLK_SYS_CTRL ) = ( 1 << 0 );
	deref( CLK_PERI_CTRL ) = ( 1 << 11 ) | ( 1 << 5 ); // CLK PERI Enable & AUX SRC = xosc_clksrc
	
	
}

inline void Wait(dword amount)
{
	for(volatile register dword wait = amount; wait--;)
	{
		
	}
}

word screen[40*16] = {32*64};
word screenB[40*16] = {32*64};
byte screenFlags[40*16] = {0};
byte screenFlagsB[40*16] = {0};
byte screenFlagsC[40*16] = {0};
dword scanline = 0;
byte syncparity = 0;
word printpos = 0;

void Print(const char* str)
{
	while(*str)
	{
		screen[printpos] = ((word)(*str))*64;
		screenB[printpos] = 32*64;
		screenFlags[printpos] = 0;
		screenFlagsB[printpos] = 0;
		screenFlagsC[printpos] = 0;
		++printpos;
		if(printpos > 40*16)
		{
			printpos = 0;
		}
		++str;
	}
}

char printdwordbuf[11];

void PrintDword(dword num)
{
	printdwordbuf[10] = '\x00';
	printdwordbuf[9] = ' ';
	byte* out = &printdwordbuf[8];
	for(int luup = 0; luup < 7; luup++)
	{
		if((num&0xf) > 0x9)
		{
			*out = 'a'+(num&0xf)-0xa;
		}
		else
		{
			*out = '0'+(num&0xf);
		}
		num = num>>4;
		if(!num)
		{
			Print(out);
			return;
		}
		out--;
	}
	if((num&0xf) > 0x9)
	{
		*out = 'a'+(num&0xf)-0xa;
	}
	else
	{
		*out = '0'+(num&0xf);
	}
	Print(out);
}

/*
static void Print(const char* str)
{
	deref(SIO_GPIO_OUT+0x4) = (1<<15) | (1<<14);
	Wait(60000);
	while(*str)
	{
		for(int i = 0; i < 8; i++)
		{
			dword time = (((*str)>>i)&1)*4500;
			deref(SIO_GPIO_OUT+0x8) = (1<<15) | (1<<14);
			Wait(4500+time);
			deref(SIO_GPIO_OUT+0x4) = (1<<15) | (1<<14);
			Wait(4500+time);
		}
		Wait(16000);
		str++;
	}
	deref(SIO_GPIO_OUT+0x8) = (1<<15) | (1<<14);
}
*/

#define DisplayPixel() deref(SIO_GPIO_OUT+0x8-((*(scrptr++))<<2)) = 1<<15;

#define VYEAH 4
#define VLENGTH (VYEAH*16)
#define VSHORT VYEAH
#define VLONG VYEAH




#define CLKPERUS 100

#define DisplayChar(num) \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+0]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+0]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+0]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+1]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+1]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+1]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+2]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+2]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+2]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+3]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+3]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+3]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+4]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+4]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+4]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+5]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+5]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+5]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+6]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+6]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+6]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+7]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+7]) = 1<<15; \
deref(SIO_GPIO_OUT + 0x4 + line[num*8+7]) = 1<<15; 

#define CalcChar(num) \
vertInv = (vertplus)^(*(++flagC)); \
f = *(++flag); \
chr = (*(vert+vertInv+(*(++scrptr))))^(*(vert+vertInv+(*(++scrBptr)))); \
chr |= (chr<<1)&(*(++flagB)); \
chr ^= f; \
line[num*8+0] = (chr&1) << 2; \
line[num*8+1] = (chr&2) << 1; \
line[num*8+2] = (chr)&4; \
line[num*8+3] = (chr>>1)&4; \
line[num*8+4] = (chr>>2)&4; \
line[num*8+5] = (chr>>3)&4; \
line[num*8+6] = (chr>>4)&4; \
line[num*8+7] = (chr>>5)&4;

static void Thirdpart()
{
	deref(SIO_GPIO_OUT+0x4) = 1<<14;
	dword scanl = scanline;
	if(scanl >= 309u)
	{
		deref( CORTEX_SYST_RVR) = CLKPERUS*32;
		deref(0x2000013c) = &ShortSync;
		syncparity = 0;
	}
	else
	{
		deref( CORTEX_SYST_RVR) = CLKPERUS*4;
		deref(0x2000013c) = &Firstpart;
	}
	
	
	
	if((scanl)>18*16 || (scanl) < 32)
	{
		(scanline)++;
		return;
	}
	int verticalchar = (((scanl)>>4)-2)*40 - 1;
	word* scrptr = &screen[verticalchar];
	word* scrBptr = &screenB[verticalchar];
	byte* const vert = _binary_image_raw_start;
	byte vertplus = (((scanl)&0b1111)<<2)+1;
	byte* flag = screenFlags+verticalchar;
	byte* flagB = screenFlagsB+verticalchar;
	byte* flagC = screenFlagsC+verticalchar;
	//byte horfidk = scanlidk>>1;
	byte f = *(++flag);
	byte vertInv = (vertplus)^(*(++flagC));
	byte chr = (*(vert+vertInv+(*(++scrptr))))^(*(vert+vertInv+(*(++scrBptr))));
	//byte chr = 0x10001100;
	chr |= (chr<<1)&(*(++flagB));
	chr ^= f;
	
	byte line[40*8] = {};
	line[0] = (chr&1) << 2;
	line[1] = (chr&2) << 1;
	line[2] = (chr)&4;
	line[3] = (chr>>1)&4;
	line[4] = (chr>>2)&4;
	line[5] = (chr>>3)&4;
	line[6] = (chr>>4)&4;
	line[7] = (chr>>5)&4;
	CalcChar(1);
	CalcChar(2);
	CalcChar(3);
	CalcChar(4);
	CalcChar(5);
	CalcChar(6);
	CalcChar(7);
	CalcChar(8);
	CalcChar(9);
	CalcChar(10);
	CalcChar(11);
	CalcChar(12);
	CalcChar(13);
	CalcChar(14);
	CalcChar(15);
	CalcChar(16);
	CalcChar(17);
	CalcChar(18);
	CalcChar(19);
	CalcChar(20);
	CalcChar(21);
	CalcChar(22);
	CalcChar(23);
	CalcChar(24);
	CalcChar(25);
	CalcChar(26);
	CalcChar(27);
	CalcChar(28);
	CalcChar(29);
	CalcChar(30);
	CalcChar(31);
	CalcChar(32);
	CalcChar(33);
	CalcChar(34);
	CalcChar(35);
	CalcChar(36);
	CalcChar(37);
	CalcChar(38);
	CalcChar(39);
	DisplayChar(0);
	DisplayChar(1);
	DisplayChar(2);
	DisplayChar(3);
	DisplayChar(4);
	DisplayChar(5);
	DisplayChar(6);
	DisplayChar(7);
	DisplayChar(8);
	DisplayChar(9);
	DisplayChar(10);
	DisplayChar(11);
	DisplayChar(12);
	DisplayChar(13);
	DisplayChar(14);
	DisplayChar(15);
	DisplayChar(16);
	DisplayChar(17);
	DisplayChar(18);
	DisplayChar(19);
	DisplayChar(20);
	DisplayChar(21);
	DisplayChar(22);
	DisplayChar(23);
	DisplayChar(24);
	DisplayChar(25);
	DisplayChar(26);
	DisplayChar(27);
	DisplayChar(28);
	DisplayChar(29);
	DisplayChar(30);
	DisplayChar(31);
	DisplayChar(32);
	DisplayChar(33);
	DisplayChar(34);
	DisplayChar(35);
	DisplayChar(36);
	DisplayChar(37);
	DisplayChar(38);
	DisplayChar(39);
	deref(SIO_GPIO_OUT + 0x8) = 1 << 15;
	(scanline)++;
	
}

dword lastseed = 0;

static dword random()
{
	lastseed = lastseed*225733+235789;
	return random;
}

static void Secondpart()
{
	deref( CORTEX_SYST_RVR) = CLKPERUS*57;
	deref(0x2000013c) = &Thirdpart;
	deref(SIO_GPIO_OUT+0x4) = 1<<14;
}

static void Firstpart()
{
	deref( CORTEX_SYST_RVR) = CLKPERUS*60;
	deref(0x2000013c) = &Thirdpart;
	deref(SIO_GPIO_OUT+0x8) = 1<<14;;
}



static void ShortSync()
{
	deref(SIO_GPIO_OUT+0x8) = 1<<14;
	if(syncparity)
	{
		if((scanline)++ >= 311)
		{
			scanline = 0;
			deref(CORTEX_SYST_RVR) = CLKPERUS*32;
			deref(0x2000013c) = &LongSync;
		}
		else if((scanline) >= 5 && (scanline) < 302)
		{
			deref(CORTEX_SYST_RVR) = CLKPERUS*4;
			deref(0x2000013c) = &Firstpart;
		}
		else
		{
			deref(CORTEX_SYST_RVR) = CLKPERUS*32;
		}
		(syncparity) = 0;
	}
	else
	{
		deref( CORTEX_SYST_RVR) = CLKPERUS*32;
		(syncparity) = 1;
	}
	
	Wait(CLKPERUS);
	deref(SIO_GPIO_OUT+0x4) = 1<<14;
}

static void LongSync()
{
	deref(SIO_GPIO_OUT+0x8) = 1<<14;
	if(syncparity)
	{
		(scanline)++;
		deref(CORTEX_SYST_RVR) = CLKPERUS*32;
		(syncparity) = 0;
	}
	else
	{
		if(scanline >= 2)
		{
			deref(0x2000013c) = &ShortSync;
		}
		deref( CORTEX_SYST_RVR) = CLKPERUS*32;
		(syncparity) = 1;
	}
	
	Wait(CLKPERUS*2);
	deref(SIO_GPIO_OUT+0x4) = 1<<14;
}

inline int abs(int i)
{
	if(i<0)
	{
		return -i;
	}
	return i;
}

#define WIFI_SPI_SPEED 500

static void SendDword(dword sent)
{
	//sent = (sent>>16) | (sent<<16);
	deref(SIO_GPIO_OE_SET) = (1<<24);
	deref(SIO_GPIO_OUT+ 0x8) = 1<<25;
	Wait(WIFI_SPI_SPEED);
	for(dword i = 0; i < 32; i++)
	{
		deref(SIO_GPIO_OUT+ 0x4) = 1<<29;
		Wait(WIFI_SPI_SPEED);
		deref(SIO_GPIO_OUT+ 0x8) = 1<<29;
		deref(SIO_GPIO_OUT+ 0x8 - ((sent&0x80000000)>>29)) = 1<<24;
		Wait(WIFI_SPI_SPEED);
		sent <<= 1;
	}
	deref(SIO_GPIO_OUT+ 0x8) = 1<<29;
	deref(SIO_GPIO_OE_CLR) = (1<<24);
	Wait(WIFI_SPI_SPEED);
}


static dword ReadDword()
{
	dword ret = 0;
	deref(SIO_GPIO_OUT+ 0x8) = 1<<25;
	Wait(WIFI_SPI_SPEED);
	for(dword i = 0; i < 32; i++)
	{
		deref(SIO_GPIO_OUT+ 0x4) = 1<<29;
		ret <<= 1;
		ret |= 1-((deref(SIO_GPIO_IN)&(1<<24))>>24);
		Wait(WIFI_SPI_SPEED);
		deref(SIO_GPIO_OUT+ 0x8) = 1<<29;
		Wait(WIFI_SPI_SPEED);
	}
	return ret;
}

static void PulseCS()
{
	deref(SIO_GPIO_OUT+ 0x4) = 1<<25;
	Wait(WIFI_SPI_SPEED);
	deref(SIO_GPIO_OUT+ 0x8) = 1<<25;
	Wait(WIFI_SPI_SPEED);
}

static void PulseCLK()
{
	deref(SIO_GPIO_OUT+ 0x4) = 1<<29;
	Wait(WIFI_SPI_SPEED);
	deref(SIO_GPIO_OUT+ 0x8) = 1<<29;
	Wait(WIFI_SPI_SPEED);
}

inline void SetTing(dword p)
{
	deref(SIO_GPIO_OUT+0x4) = (1<<14)|(1<<15);
	for(volatile dword i = 0; i < p; i++)
	{
		
	}
	deref(SIO_GPIO_OUT+0x8) = (1<<14)|(1<<15);
	for(volatile dword i = 0; i < 0x30-p; i++)
	{
		
	}
}

extern void wifi_spi_init2();
extern void wifi_spi_transfer2(byte* send, dword send_len, byte* recv, dword recv_len);

static inline dword make_cmd(dword write, dword inc, dword fn, dword addr, dword sz) {
    return (write << 31) | (inc << 30) | (fn << 28) | ((addr & 0x1ffff) << 11) | sz;
}

void wifi_set_backplane_window(dword addr)
{
	addr &= ~0x7fffu;
	addr >>= 8;
	
	static dword lastbackplaneaddr = 0;
	if(lastbackplaneaddr != addr)
	{
		dword msg[2];
		msg[0] = make_cmd(1,1,1,0x1000a,3);
		msg[1] = addr;
		wifi_spi_transfer2(msg,8,0,0);
		lastbackplaneaddr = addr;
	}
}

void wifi_write_backplane(dword addr, dword val, dword size)
{
	wifi_set_backplane_window(addr);
	dword msg[2];
	if(size == 4)
	{	
		msg[0] = make_cmd(1,1,1,(addr&0x7fff) | 0x8000,size);
	}
	else
	{
		msg[0] = make_cmd(1,1,1,(addr&0x7fff),size);
	}
	msg[1] = val;
	wifi_spi_transfer2(msg,8,0,0);
}

dword wifi_read_backplane(dword addr, dword size)
{
	wifi_set_backplane_window(addr);
	dword msg[6];
	if(size == 4)
	{	
		msg[0] = make_cmd(0,1,1,(addr&0x7fff) | 0x8000,size);
	}
	else
	{
		msg[0] = make_cmd(0,1,1,(addr&0x7fff),size);
	}
	wifi_spi_transfer2(msg,4,&msg[1],20);
	return msg[5];
}


void wifi_reset(dword ram_or_arm)
{
	dword msg[6];
	wifi_read_backplane(0x18000000+ram_or_arm+0x100000+0x408,1);
	
	
	//PrintDword(msg[2]);
	
	
	wifi_write_backplane(0x18000000+ram_or_arm+0x100000+0x408,3,1);

	//PrintDword(msg[2]);
	
	
	wifi_read_backplane(0x18000000+ram_or_arm+0x100000+0x408,1);
	
	//PrintDword(msg[2]);
	
	
	wifi_write_backplane(0x18000000+ram_or_arm+0x100000+0x800,0,1);
	
	
//PrintDword(msg[2]);
	

	Wait(100000);
	
	
	wifi_write_backplane(0x18000000+ram_or_arm+0x100000+0x408,1,1);
	
	
	//PrintDword(msg[2]);
	
	
	wifi_read_backplane(0x18000000+ram_or_arm+0x100000+0x408,1);
	
	
	//PrintDword(msg[2]);
	
	
	Wait(10000);
}



__attribute__( ( used, section( ".boot.entry" ) ) ) void main()
{
	clock_init();
	
	Wait(500000);
	
	*(volatile dword*)(RESETS_RESET) &= ~(1<<5);
	while( (deref(RESETS_RESET_DONE) & (1<<5)) == 0 )
	{
		
	}
	
	*(volatile dword*)(RESETS_RESET) &= ~(1<<10);
	while( (deref(RESETS_RESET_DONE) & (1<<10)) == 0 )
	{
		
	}
	
	*(volatile dword*)(RESETS_RESET) &= ~(1<<2);
	while( (deref(RESETS_RESET_DONE) & (1<<2)) == 0 )
	{
		
	}
	
	scanline = 0;
	byte holyshit = 0;
	byte holyshit2 = 0;
	char holyshit3 = 0; // needs to be signed i think
	
	deref(IO_BANK0_GPIO(14)) = 5; //SIO
	deref(IO_BANK0_GPIO(15)) = 5; //SIO
	deref(IO_BANK0_GPIO(24)) = 5; //SIO
	deref(IO_BANK0_GPIO(25)) = 5; //SIO
	deref(IO_BANK0_GPIO(23)) = 5; //SIO
	deref(IO_BANK0_GPIO(29)) = 5; //SIO
	//deref(IO_BANK0_GPIO(25)) = 5; //SIO
	//deref(IO_BANK0_GPIO(24)) = 5; //SIO
    deref(SIO_GPIO_OE_SET) = /*(1<<25)+(1<<29)+*/(1<<29)+(1<<24)+(1<<25)+(1<<23)+(1<<14)+(1<<15);
	deref(0x4001c068) = 0b01010001;
	deref(0x4001c064) = 0b01010001;
	deref(0x4001c060) = 0b01010001;
	deref(0x4001c078) = 0b01010001;
	//deref(0x4001c060) = 0b01111001;
	//deref(SIO_GPIO_OUT+0x8) = 1<<29;
	//deref(SIO_GPIO_OUT+0x8) = 1<<23;
	//deref(SIO_GPIO_OUT+0x4) = 1<<25;
	//Wait(200);
	//deref(SIO_GPIO_OUT+0x4) = 1<<23;
	//Wait(3600000);
	//Wait(CLKPERUS*5000);
	
	
	
	//deref(0x4005802c) = 1+(1<<9);
	//deref(0x40054038+0x1000) = 0b1111;
	//deref( CORTEX_BASE + 0xe280) = 0b1111;
	//deref( CORTEX_BASE + 0xe100) = 0b1111;
	//deref(0x40054010) = deref(0x40054028)+5*12;
	//deref(0x4005402c) = 0b0;
	//deref(0x40054030) = 0b0;
	for(dword i = 0; i < 16; i++)
	{
		for(dword j = 0; j < 40; j++)
		{
			screen[i*40+j] = 32*64;
			screenB[i*40+j] = 32*64;
			screenFlags[i*40+j] = 0x0u;
			screenFlagsB[i*40+j] = 0x0u;
			screenFlagsC[i*40+j] = 0x0u;
		}
	}
	
    deref( CORTEX_SYST_RVR) = CLKPERUS*32;
	deref( CORTEX_SYST_CSR) = ( 1 << 2 ) | ( 1 << 1 ) | ( 1 << 0 );
	wifi_spi_init2();
	
	dword msg[6];
	msg[0] = make_cmd(0,1,0,0,4);
	msg[1] = 0;
	wifi_spi_transfer2(msg,4,&msg[1],4);
	
	
	wifi_spi_transfer2(msg,4,&msg[1],4);
	
	msg[0] = make_cmd(1,1,0,0x1d,1);
	msg[1] = 16;
	wifi_spi_transfer2(msg,8,0,0);
	
	msg[0] = make_cmd(1,1,0,0x4,2);
	msg[1] = 0x0;
	wifi_spi_transfer2(msg,8,0,0);
	
	
	msg[0] = make_cmd(1,1,0,0x6,2);
	msg[1] = 0x0;
	wifi_spi_transfer2(msg,8,0,0);
	
	
	
	msg[0] = make_cmd(1,1,1,0x1000e,1);
	msg[1] = 8;
	wifi_spi_transfer2(msg,8,0,0);


	msg[0] = make_cmd(0,1,1,0x1000e,1);
	msg[1] = 0;
	
	Print("|");
	
	wifi_spi_transfer2(msg,4,&msg[1],20);
	while(!(msg[5] & 0x40))
	{
		wifi_spi_transfer2(msg,4,&msg[1],20);
		Wait(10000);
	}
	
	Print("|");
	
	msg[0] = make_cmd(1,1,1,0x1000e,1);
	msg[1] = 0;
	wifi_spi_transfer2(msg,8,0,0);
	
	
	
	
	
	wifi_reset(0x4000);
	
	
	
	
	
	
	wifi_write_backplane(0x18004000+0x10,3,4);
	
	
	wifi_write_backplane(0x18004000+0x44,0,4);
	
	
	

	
	PrintDword(wifi_read_backplane(0x100,4));
	
	
	
	
	msg[0] = make_cmd(1,1,1,0x1000a,3);
	msg[1] = 0x0;
	wifi_spi_transfer2(msg,8,0,0);
	
	Wait(10000);
	
	
	for(dword i = 0; i < wifi_code_compiled_bin_len; i+=4)
	{
		wifi_write_backplane(i,*(dword*)(&wifi_code_compiled_bin[i]),4);
	}
	
	PrintDword(wifi_read_backplane(0x100+18,4));

	Wait(10000);

	PrintDword(wifi_read_backplane(0x380,4));
	wifi_reset(0x3000);
	PrintDword(wifi_read_backplane(0x380,4));
	//Wait(10000);
	while(1)
	{
		for(dword i = 0; i < 40*16; i ++)
		{
			screen[i] = (wifi_read_backplane(0x1000+i,1)&0xff)*64;
		}
	}
	
	
	

	
	//Print("\x19\x14\x14testbutton\x14\x14\x04");
	/*
	SendDword((1<<31)|4);
	SendDword(0b10001001);
	PrintDword(ReadDword());
	PulseCS();
	Print(" ");
	SendDword(0);
	PulseCS();
	PrintDword(ReadDword());
	PrintDword(ReadDword());
	PulseCS();
	Print(" ");
	SendDword(0);
	PulseCS();
	PrintDword(ReadDword());
	PrintDword(ReadDword());
	PulseCS();
	Print(" ");
	SendDword(0);
	PulseCS();
	PrintDword(ReadDword());
	PrintDword(ReadDword());
	PulseCS();
	Print(" ");
	deref(SIO_GPIO_OUT+0x4) = 1<<25;
	Wait(WIFI_SPI_SPEED);
	*/
	//dword i = 0;
	while (1)
    {
		
		/*
		for(dword i = 0; i < 16; i++)
		{
			for(dword j = 0; j < 40; j++)
			{
				screen[i*40+j] = (((screen[i*40+j]>>2)+1)<<2)&16383;
				screenB[i*40+j]  = (((screenB[i*40+j]>>2)+3)<<2)&16383;
				screenFlagsC[i*40+j]=(screenFlagsC[i*40+j]+6)&0b11111100;
				//screenFlags[i*40+j]+=0;
				//screenFlagsB[i*40+j]+=0;
			}
		}
		Wait(217800);
		*/
	}
	/*
    while (1)
    {
		for(dword yeah = 0; yeah < 10; yeah++)
		{
			for(dword i = 0; i < 16; i++)
			{
				for(dword j = 0; j < 40; j++)
				{
					screen[i*40+j] = ((i+j+holyshit+64)&0xff)*64+((((abs(j-20)*abs(holyshit3-127))>>4))&0xff)*4;
					screenFlags[i*40+j] = 0x0u;
					screenFlagsB[i*40+j] = 0x0u;
				}
				holyshit3++;
			}
			holyshit2++;
			Wait(100000);
		}
		holyshit++;
		
		//char hi[5];
		//dword yeah = deref(0x40054034)&0b1111;
		//dword yeah = deref(0x4005402c);
		//hi[0] = 0b10101010;
		//hi[1] = yeah&0xff;
		//hi[2] = (yeah&0xff00)>>8;
//hi[3] = (yeah&0xff0000)>>16;
		//hi[4] = '\x00';
		//Print(hi);


        // loop here and wait for the tick interrupt
    }
	*/
	/*
	
	deref(0x4000c000u | (2u<<12u)) = 0x01000000;
	deref(0x4000c000u | (3u<<12u)) = 0x01000000;
	while(!(deref(0x4000c000u + 0x8)&0x01000000))
	{
		
	}
	
	
	deref(CLEAR_PENDING_INT) = 1<<5;
	deref(SET_ENABLE_INT) = 1<<5;
	usb_regs->muxing = 0b1001;
	usb_regs->pwr = 0b1100;
	usb_regs->main_ctrl = 1;
	usb_regs->sie_ctrl = 0x20000000u;
	usb_regs->inte = 0x00011010u;
	usb_device->ep_ctrl[0].out = ((dword)&((usb_device_dpram_t*)0)->epx_data[0]) | (1u << 31) | (1u << 29) | (2u << 26);
	usb_device->ep_ctrl[1].in = ((dword)&((usb_device_dpram_t*)0)->epx_data[64]) | (1u << 31) | (1u << 29) | (2u << 26);
	((usb_regs_t*)(((dword)usb_regs)|REG_ALIAS_CLR_BITS))->sie_ctrl = 0x00010000u;
	while(1)
	{
		
	}
	*/
}
	/*
	
	
	static void Display()
{
	
	
	deref(SIO_GPIO_OUT) = 0; Wait(VLENGTH-VLONG); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLONG);
	deref(SIO_GPIO_OUT) = 0; Wait(VLENGTH-VLONG); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLONG);
	deref(SIO_GPIO_OUT) = 0; Wait(VLENGTH-VLONG); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLONG);
	deref(SIO_GPIO_OUT) = 0; Wait(VLENGTH-VLONG); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLONG);
	deref(SIO_GPIO_OUT) = 0; Wait(VLENGTH-VLONG); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLONG);
	
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	
	Wait(30);
	
	for(dword i = 0; i != 304; ++i)
	{
		deref(SIO_GPIO_OUT) = 0;
		scrptr = screen + (i<<7);
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 0;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; ++scrptr;
		deref(SIO_GPIO_OUT) = *scrptr; 
		
		
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
		deref(SIO_GPIO_OUT) = 1 << 14;
	}
	
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	deref(SIO_GPIO_OUT) = 0; Wait(VSHORT); deref(SIO_GPIO_OUT) = 1 << 14; Wait(VLENGTH-VSHORT);
	
}
	
	*/