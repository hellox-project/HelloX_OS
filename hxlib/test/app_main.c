/* Demonstrated application code using HelloX lib. */
#include <hellox.h>
#include <stdio.h>
#include <string.h>

/* A short poem used as flash message. */
char* the_poem = "The quiet dawn breaks through the pines,\n"
"A golden light on waking lines.\n"
"The river hums a soft, low tune\n"
"Beneath the silver eye of moon.\n"
"The world still dreams in dew and gray,\n"
"And I just breathe and start the day.\n";

int exam_main(int argc, char* argv[])
{
	_hx_printf("Begin to put flash message...\r\n");
#if 1
	if (PutFlashMessage(NULL,
		strlen(the_poem) + 1,
		the_poem))
	{
		_hx_printf("Put flash message OK.\r\n");
		return 0;
	}
	else {
		_hx_printf("Failed to put flash message.\r\n");
		return -1;
	}
#endif
	return 0;
}
