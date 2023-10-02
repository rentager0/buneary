OUTPUT_FORMAT(elf64-x86-64)
ENTRY(entry)

SECTIONS
{
  . = 0xffffffff80000000;
  
  .text : ALIGN(0x1000) {
    PROVIDE(__kernel_text_start = .);
    *(.multiboot2);
    *(.text);
    PROVIDE(__kernel_text_end = .);
  }

  .data : {
    PROVIDE(__kernel_data_start = .);
    *(.data);
    *(.rodata*);
    PROVIDE(__kernel_data_end = .);
  }

  .bss : {
    PROVIDE(__kernel_bss_start = .);
    *(.bss);
  }
  PROVIDE(__kernel_bss_end = .);

  /DISCARD/ : {
    *(.eh_frame .comment);
  }
}
