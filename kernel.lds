OUTPUT_ARCH(riscv)

ENTRY(_start)

SECTIONS {
    kernel_start = .;

    start 0x80200000 : { init/start.o }

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
    . = 0x80600000;
    end = . ;
}