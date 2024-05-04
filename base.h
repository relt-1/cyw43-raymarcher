#ifndef BASE_H
#define BASE_H

typedef unsigned int dword;

typedef unsigned short word;

typedef unsigned char byte;

// Shortcut for address access
#define deref(X) (*(volatile dword*)(X))

#define INTERNAL_REGISTERS 0xe0000000u

#define SET_ENABLE_INT (INTERNAL_REGISTERS+0xe100u)
#define CLEAR_ENABLE_INT (INTERNAL_REGISTERS+0xe180u)
#define SET_PENDING_INT (INTERNAL_REGISTERS+0xe200u)
#define CLEAR_PENDING_INT (INTERNAL_REGISTERS+0xe280u)

#define INT_TABLE (INTERNAL_REGISTERS+0xed08u)

#define REG_ALIAS_XOR_BITS (0x1u << 12u)
#define REG_ALIAS_SET_BITS (0x2u << 12u)
#define REG_ALIAS_CLR_BITS (0x3u << 12u)

#endif