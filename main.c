#include <htc.h>

__CONFIG( \
	INTIO \		/* Use internal oscillator without CLKOUT */
	& WDTDIS \	/* Disable watch dog timer */
	& PROTECT \	/* Protect program code */
);

int
main(int argc, char *argv[])
{
	while (1) {
		/* LOOP: Wait for interrupt */
	}
}
