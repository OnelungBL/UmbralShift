/* sample.c - Sample Main program for SPEDE
Darren Takemoto
CSC 159 - Spring 2016
Phase 0 - Lab 2
*/

#include <spede/stdio.h> /* For printf() */
#include <spede/flames.h> /* For cons_prinf() */

//void DisplayMsg(int);

int main(void) {
	int i;
	int x;
	i=111;
	for(x=0; x<5; x++) {
		printf("%d Hello world %d \nECS", i, 2 * i);
		cons_printf("--> Hello world <--\nCPE/CSC");
		//DisplayMsg(i);
		i++;
	}
	return 0;
} /* end main() */

//void DisplayMsg(int i) {
//	printf("%d Hello world %d \nECS", i, 2 * i);
//	cons_printf("--> Hello world <--\nCPE/CSC");
//}

/* SPEDE makefile helper program
   FROM shell promp: " spede-mkmf -q
*/
