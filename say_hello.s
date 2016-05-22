.text
.global _start

_start:

   movl	$msg, %ebx

   pushl %ebx
   pushl %ebx

   popl %eax
   int $54

   int $48
   popl %eax
   int $55

   movl 8(%ebx), %eax
   int $58

.data
msg:
	.long 0
	.long 5
	.long 1
	.ascii "UmbralShift says \"Hello!\"\n\0"
	.rept 75
		.ascii "\0"
	.endr
	.long 0
	.long 0
	.long 0
