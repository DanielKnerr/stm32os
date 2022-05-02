MEMORY {
    /* siehe Table 4. "Flash module organization" im STM32F411 Reference Manual */
    FLASH (rx) : ORIGIN = 0x8000000,  LENGTH = 512K

    /* Zitat aus STM32F411 Reference Manual: "the data area (SRAM) starts from address 0x2000 0000" */
    RAM (! rx) : ORIGIN = 0x20000000, LENGTH = 128K
}

ENTRY(_reset)

SECTIONS {
    /* der vector table muss ganz am Anfang im Flash sein, da dort auch der Reset Vector ist */
    .int_vector_table : {
        *(.int_vector_table)
    } > FLASH

    /* danach kommt der Code */
    .text : {
        . = ALIGN(4);
        *(.text .text.* .gnu.linkonce.t.*)
    } > FLASH

    /* und danach die Daten */
    .data : {
        . = ALIGN(4);
        _data_lma = LOADADDR(.data);
        _data_vma = .;
        *(.data)
        *(.rodata)
        . = ALIGN(4);
        _data_size = SIZEOF(.data);
    } > RAM AT >FLASH

    .bss : {
        . = ALIGN(4);
        _bss_lma = LOADADDR(.bss);
        _bss_vma = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _bss_size = SIZEOF(.bss);
    } > RAM AT >FLASH
}

_estack = 0x20020000;