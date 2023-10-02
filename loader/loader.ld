OUTPUT_ARCH(i386)
OUTPUT_FORMAT(elf32-i386)
ENTRY(entry_multiboot2)

SECTIONS
{
  . = 0x100000;
  
  .text : ALIGN(0x1000) {
    __kernel_text_start = .;
    *(.bootstrap);
    *(.mistral);
    *(.multiboot2);
    *(.text*);
  }
  
  .jumper : {
    __kernel_jumper_start = .;
    *(.jumper*);
  }

  __kernel_text_end = .;

  .data : {
    __kernel_data_start = .;
    *(.data);
    *(.rodata*);
    *(.bss);
    *(COMMON);
  }

  __kernel_data_end = .;
  __kernel_bss_end = .;

  /DISCARD/ : {
    *(.eh_frame .comment);
  }
}
