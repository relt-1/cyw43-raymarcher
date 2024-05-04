#ifndef GPIO_H
#define GPIO_H

#define IO_BANK0_BASE 0x40014000UL
#define IO_BANK0_GPIO(x) (IO_BANK0_BASE + x*8 + 0x04)


#define SIO_BASE 0xD0000000UL
#define SIO_GPIO_IN (SIO_BASE + 0x04)
#define SIO_GPIO_OUT (SIO_BASE + 0x10)
#define SIO_GPIO_OUT_SET (SIO_BASE + 0x14)
#define SIO_GPIO_OUT_CLR (SIO_BASE + 0x18)
#define SIO_GPIO_OUT_XOR (SIO_BASE + 0x1c)
#define SIO_GPIO_OE (SIO_BASE + 0x20)
#define SIO_GPIO_OE_SET (SIO_BASE + 0x24)
#define SIO_GPIO_OE_CLR (SIO_BASE + 0x28)

#endif