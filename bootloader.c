#include "base.h"

#define XIP_SSI_BASE       0x18000000
#define XIP_SSI_CTRLR0     XIP_SSI_BASE + 0x00
#define XIP_SSI_CTRLR1     XIP_SSI_BASE + 0x04
#define XIP_SSI_SSIENR     XIP_SSI_BASE + 0x08
#define XIP_SSI_BAUDR      XIP_SSI_BASE + 0x14
#define XIP_SSI_SPI_CTRLR0 XIP_SSI_BASE + 0xF4



// IO Bank
#define IO_BANK0_BASE 0x40014000UL
#define IO_BANK0_GPIO00_CTRL (IO_BANK0_BASE + 0x04)

// SIO
#define SIO_BASE 0xD0000000UL
#define SIO_GPIO_OUT (SIO_BASE + 0x10)
#define SIO_GPIO_OUT_SET (SIO_BASE + 0x14)
#define SIO_GPIO_OUT_CLR (SIO_BASE + 0x18)
#define SIO_GPIO_OUT_XOR (SIO_BASE + 0x1c)
#define SIO_GPIO_OE (SIO_BASE + 0x20)
#define SIO_GPIO_OE_SET (SIO_BASE + 0x24)
#define SIO_GPIO_OE_CLR (SIO_BASE + 0x28)

#define RESETS_BASE 0x4000C000UL
#define RESETS_RESET (RESETS_BASE + 0x00)
#define RESETS_RESET_DONE (RESETS_BASE + 0x08)



__attribute__( (  noreturn, used, naked,  retain, section( ".boot2" ) ) ) void boot2() 
{
	
	deref(XIP_SSI_SSIENR) = 0x00000000;
	deref(XIP_SSI_BAUDR) = 0x00000004;
	deref(XIP_SSI_CTRLR0) = 0x001F0300;
	deref(XIP_SSI_SPI_CTRLR0) = 0x03000218;
	deref(XIP_SSI_CTRLR1) = 0x00000000;
	deref(XIP_SSI_SSIENR) = 0x00000001;
	
	//for(deref(0x20000000) = 0; deref(0x20000000) < 0x1000; deref(0x20000000)++)
	//{
	//	deref(0x20000100+deref(0x20000000)*4) = deref(0x10000100+deref(0x20000000)*4);
	//}
	deref(INT_TABLE) = 0x20000100;
	
	__asm__("ldr r0,=0x20042000\n"
			"msr msp,r0");
	
	// Jump
	__asm__("ldr r0, =0x20000201\n"
			"bx  r0");
    
	
	
	// ... and never return.
	while(1)
	{
		
	}
}