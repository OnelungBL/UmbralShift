/* sample.c - Sample Main program for SPEDE
Darren Takemoto
CSC 159 - Spring 2016
Phase 0 - Lab 1
*/

#include <spede/stdio.h> /* For printf() */
#include <spede/flames.h> /* Fo cons_prinf() */

int main(void) {
	int i;
	
	i=128;
	printf("%d Hello world %d \nECS", i, 2 * i);
	cons_printf("--> Hello world <--\nCPE/CSC");
	return 0;
} /* end main() */

/* SPEDE makefile helper program
   FROM shell promp: " spede-mkmf -q
*/
