OUTPUT_FORMAT("elf32-littlearm", "elf32-bigram", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
	RAM (rwx): org = 0x08000000,
	        len = 1M
	FLASH (rx): org = 0x00000000,
		len = 1M 
}

SECTIONS {
	.text : {
		_start = .;
		bootloader.o(.text)
		*(.text)
	} > FLASH

	.rodata : {
		*(.rodata)
	} > FLASH

	.data : {
		_start_data = .;
		*(.data)
		_end_data = .;
	} > RAM AT>FLASH

	data_flash = LOADADDR(.data);
	data_size = _end_data -  _start_data;

	.bss : {
		_start_bss = .;
		*(.bss)
		_end_bss = .;
	} > RAM
	bss_size = _end_bss -  _start_bss;

	start_stack = 0x09000000;
}
