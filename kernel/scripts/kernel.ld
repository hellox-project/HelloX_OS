ENTRY(__OS_Entry)
SECTIONS
{
	/* 段起始位置 */
	/*. = 0x10ffd0;*/
	.text 0x110000:
	{
		*(.text)
		. = ALIGN(4096);
	}

}
