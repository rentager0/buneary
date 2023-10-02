typedef unsigned short int uint16_t;
typedef unsigned int       uint32_t;

struct multiboot2_info
{
	uint32_t total_size;
	uint32_t reserved;
};

void main(struct multiboot2_info *info, uint32_t magic)
{
	uint16_t *vga = (uint16_t *) 0xb8000;
	
	if (info && magic) {}

	vga[0] = 0x700 | 'H';
	vga[1] = 0x700 | 'E';
	vga[2] = 0x700 | 'L';
	vga[3] = 0x700 | 'L';
	vga[4] = 0x700 | 'O';
	vga[5] = 0x700 | ' ';
	vga[6] = 0x700 | '!';
	
	while (1)
		;
}
