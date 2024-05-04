MEMORY
{
    RAM(rx) : ORIGIN = 0x00000000, LENGTH = 512k
    SHARED(rx) : ORIGIN = 0x0001000, LENGTH = 512k
}
SECTIONS
{
    .text : {
        . = ORIGIN(RAM);
		
		__int_table_start__ = .;
		KEEP(*(.int.table))
		__int_table_end__ = .;
		
        KEEP(*(.boot.entry))
        KEEP(*(.text*))
		
		
		. = ORIGIN(SHARED);
		KEEP(*(.shared))
		
        __end_code_ = .;
    } > RAM
	
	ASSERT(__int_table_end__ - __int_table_start__ == 256,
        "ERROR: Interrupt table must be 256 bytes in size")
}
