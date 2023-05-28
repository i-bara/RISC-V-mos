OUTPUT_ARCH(riscv)

ENTRY(_start)

SECTIONS {
    kernel_start = .;

    . = 0x00000000801f0000;
	


    start 0x0000000080200000 : { init/start.o }

    .text : {    
        *(.boot)
        *(.text)
    }

    .data : {
        *(.data .data* .sdata .sdata*)
    }

    .bss  : {
        *(.bss .bss* .sbss .sbss*)
    }
    bss_end = .;

    . = 0x0000000081000000;
    exc_gen_entry = .;
    .exc_gen_entry : { *(.text.exc_gen_entry) }

    . = 0x0000000081100000;
    end = .;
}