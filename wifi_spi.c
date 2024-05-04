#include "base.h"
#include "pio.h"
#include "gpio.h"
#include "DMA.h"


inline dword SWAP32(dword A)
{
	return ((((A) & 0xff000000U) >> 8) | (((A) & 0xff0000U) << 8) | (((A) & 0xff00U) >> 8) | (((A) & 0xffU) << 8));
}

inline static dword __swap16x2(dword a) {
    __asm ("rev16 %0, %0" : "+l" (a) : : );
    return a;
}

static inline dword make_cmd(dword write, dword inc, dword fn, dword addr, dword sz) {
    return (write << 31) | (inc << 30) | (fn << 28) | ((addr & 0x1ffff) << 11) | sz;
}

word spicode[] = 
{
	0b0110000000000001u,
	0b0001000001000001u,
	0b1110000010000000u,
	0b1011000001000010u,
	0b0100000000000001u,
	0b0001000010000100u,
	0b0000000000000000u
	
	
	
};

void wifi_spi_transfer(byte* send, dword send_len, byte* recv, dword recv_len)
{
	
	deref(SIO_GPIO_OUT+ 0x8) = 1<<25;
	if(recv != 0)
	{
		if(send == 0)
		{
			send = recv;
		}
		//PrintDword(*(dword*)recv);
		deref(PIO_0) &= ~1;
		deref(PIO_0+0xcc) = (6<<12) | (1<<30);
		deref(PIO_0+0xd0+0x1000) = (1<<31);
		deref(PIO_0+0xd0+0x1000) = (1<<31);
		deref(PIO_0+0xd8) = 0b1110000010000001u;
		deref(PIO_0+0x2000) = 1<<4;
		deref(PIO_0+0x2000) = 1<<8;
		deref(PIO_0+0x10) = send_len*8-1;
		deref(PIO_0+0xd8) = 0b0110000000100000u;
		deref(PIO_0+0x10) = (recv_len-send_len)*8-1;
		deref(PIO_0+0xd8) = 0b0110000001000000u;
		deref(PIO_0+0xd8) = 0b0000000000000000u;
		//PrintDword(deref(PIO_0+0xd8));
		
		deref(DMA+0x444) = 1;
		while(deref(DMA+0x10)&(1<<24));
		deref(DMA+0x444) = 2;
		while(deref(DMA+0x50)&(1<<24));
		deref(DMA) = send;
		deref(DMA+0x4) = PIO_0+0x10;
		deref(DMA+0x8) = send_len/4;
		deref(DMA+0xc) = (1<<22)|(1<<4)|(0<<11)|(2<<2)|(1);
		
		deref(DMA+0x40) = PIO_0+0x20;
		deref(DMA+0x4+0x40) = recv+send_len;
		deref(DMA+0x8+0x40) = recv_len/4 - send_len/4;
		deref(DMA+0xc+0x40) = (1<<22)|(1<<5)|(1<<11)|(2<<2)|(1)|(4<<15);
		
		deref(PIO_0) |= 1;
		__asm volatile ("" : : : "memory");
		
		while(deref(DMA+0x10)&(1<<24));
		while(deref(DMA+0x50)&(1<<24));
		
		__asm volatile ("" : : : "memory");
		
		//Print("    ");
		//PrintDword(deref(PIO_0+0x3c));
		//PrintDword(deref(PIO_0+0x40));
		//Print("    ");
		//PrintDword(deref(PIO_0+0x04));
		//PrintDword(deref(PIO_0+0x08));
		//PrintDword(deref(PIO_0+0x0c));
		//PrintDword(deref(PIO_0+0x44));
		//PrintDword(deref(PIO_0+0xcc));
		//PrintDword(deref(PIO_0+0xd8));
		
		//PrintDword(*(dword*)recv);

		
		for(dword i = 0; i < send_len; i++)
		{
			recv[i] = 0;
		}
		
	}
	else if(send != 0)
	{

		
	}
	
	
	deref(PIO_0+0xd8) = 0b1010000000000011u;
	deref(SIO_GPIO_OUT+ 0x4) = 1<<25;
}



dword read_reg_u32_swap(dword fn, dword reg) {
    dword buf[2] = {0,0};
    buf[0] = SWAP32(make_cmd(0, 1, fn, reg, 4));
    wifi_spi_transfer(0, 4, (byte *)buf, 8);
    return SWAP32(buf[1]);
}

extern void PrintDword(dword num);
extern void Print(const char*);

void wifi_spi_init()
{
	//deref(SIO_GPIO_OE_SET) = (1<<24);
	//deref(SIO_GPIO_OUT+ 0x4) = 1<<24;
	deref(SIO_GPIO_OUT+ 0x8) = 1<<23;
	for(volatile int wait = 0; wait < 20000; wait++);
	deref(SIO_GPIO_OUT+ 0x4) = 1<<23;
	for(volatile int wait = 0; wait < 2500000; wait++);
	//deref(SIO_GPIO_OUT+ 0x8) = 1<<24;
	deref(IO_BANK0_GPIO(24)) = 6;
	deref(PIO_INST(0,0)) = spicode[0];
	deref(PIO_INST(0,1)) = spicode[1];
	deref(PIO_INST(0,2)) = spicode[2];
	deref(PIO_INST(0,3)) = spicode[3];
	deref(PIO_INST(0,4)) = spicode[4];
	deref(PIO_INST(0,5)) = spicode[5];
	deref(PIO_INST(0,6)) = spicode[6];
	deref(PIO_0+0xc8) = 2<<8;
	deref(PIO_0+0xdc) = (1<<29) | (24<<5) | (1<<26) | (24) | (24<<15) | (29<<10);
	deref(PIO_0+0xd0) = (1<<16) | (1<<17);
	deref(PIO_0+0x38) = (1<<24);
	deref(PIO_0+0xd8) = 0b1110000000000001u;
	dword WHY = 0;
	while(WHY != 0xFEEDBEAD)
	{
		WHY = read_reg_u32_swap(0,0x14);
		PrintDword(WHY);
		//PrintDword(deref(PIO_0+0x040));
		for(volatile dword wait = 400000; wait--;);
	}
	
	
}








inline void Wait(dword amount)
{
	for(volatile register dword wait = amount; wait--;)
	{
		
	}
}




void wifi_spi_transfer2(byte* send, dword send_len, byte* recv, dword recv_len)
{
	deref(SIO_GPIO_OUT + 0x8) = 1<<25;
	deref(SIO_GPIO_OUT + 0x8) = 1<<29;
	deref(SIO_GPIO_OE_SET) = (1<<24);
	for(dword i = 0; i < send_len; i++)
	{
		byte val = send[i];
		for(dword j = 0; j < 8; j++)
		{
			deref(SIO_GPIO_OUT + 0x8 - (((val)&0x80)>>5)) = 1<<24;
			deref(SIO_GPIO_OUT + 0x4) = 1<<29;
			val <<= 1;
			deref(SIO_GPIO_OUT + 0x8) = 1<<29;
		}
	}
	deref(SIO_GPIO_OUT + 0x8) = 1<<24;
	deref(SIO_GPIO_OE_CLR) = (1<<24);
	for(dword i = 0; i < recv_len; i++)
	{
		byte ret = 0;
		for(dword j = 0; j < 8; j++)
		{
			ret <<= 1;
			deref(SIO_GPIO_OUT + 0x4) = 1<<29;
			ret |= (deref(SIO_GPIO_IN)&(1<<24))>>24;
			deref(SIO_GPIO_OUT + 0x8) = 1<<29;
		}
		recv[i] = ret;
	}
	deref(SIO_GPIO_OUT + 0x4) = 1<<25;
}





void wifi_spi_init2()
{
	deref(SIO_GPIO_OE_SET) = (1<<24);
	deref(SIO_GPIO_OE_SET) = (1<<29);
	deref(SIO_GPIO_OE_SET) = (1<<23);
	deref(SIO_GPIO_OE_SET) = (1<<25);
	deref(SIO_GPIO_OUT + 0x8) = 1<<24;
	deref(SIO_GPIO_OUT + 0x8) = 1<<29;
	deref(SIO_GPIO_OUT + 0x8) = 1<<23;
	deref(SIO_GPIO_OUT + 0x8) = 1<<25;
	for(volatile int wait = 0; wait < 20000; wait++);
	deref(SIO_GPIO_OUT + 0x4) = 1<<23;
	for(volatile int wait = 0; wait < 250000; wait++);
	dword buf[2] = {0,0};
    
	wifi_spi_transfer2(buf,4,&buf[1],4);
	PrintDword(buf[1]);
	dword i = 0;
	dword feedbead = 0;
	while(feedbead != 0xfeedbead)
	{
		for(volatile int wait = 0; wait < 2000; wait++);
		buf[0] = SWAP32(make_cmd(0, 1, 0, 0x14, 4));
		wifi_spi_transfer2(buf,4,&buf[1],4);
		feedbead = SWAP32(buf[1]);
	}
	
	buf[0] = SWAP32(make_cmd(1, 1, 0, 0, 4));
	buf[1] = SWAP32(0x200b3);
	
	wifi_spi_transfer2(buf,8,&buf[1],0);
	
	feedbead = 0;
	while(feedbead != 0xfeedbead)
	{
		for(volatile int wait = 0; wait < 2000; wait++);
		buf[0] = make_cmd(0, 1, 0, 0x14, 4);
		wifi_spi_transfer2(buf,4,&buf[1],4);
		feedbead = buf[1];
		PrintDword(feedbead);
	}
	
	
	
}