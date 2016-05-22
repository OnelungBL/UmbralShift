.text
.global _start

_start:             # instructions begin
	movl $1, (0x80001000)
	movl $2, (0x80002000)
	movl $3, (0x80003000)
	movl $4, (0x80004000)
	int  $58         # call exit interrupt

.data
