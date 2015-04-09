#include "colog.h"

/*
 * based on https://github.com/thlorenz/log.h/blob/master/log.h
 * colour info taken from https://wiki.archlinux.org/index.php/Color_Bash_Prompt
 */

void colog(unsigned int font, unsigned int foreground, const char *format, ...)
{
	va_list ap;
	unsigned int background;

	if (font == CF_INVERSE) {
		background = foreground + CA_BACKGROUND;
		foreground = C_BLACK;
		font = font & ~CF_INVERSE;
	} else {
		background = 40;
	}

	// set colour
	fprintf(stderr, "\e[%d;%dm\e[%dm", font, foreground, background);

	// print mesg
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	// reset colour
	fprintf(stderr, "\e[0m");
}
