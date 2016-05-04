# say_hello.s
#
# In order to MsgSnd(&my_msg)/MsgRcv(&my_msg), 1st calc DRAM addr of my_msg.
#
# As program runs, subtract 4096 from ESP to get where the DRAM starts.
# $my_msg is given location 2G+x (a virtual address given by gcc link386).
# Subtract 2G from $my_msg we get x.
# Add x to the actual beginning of DRAM, that's where my_msg really is.

.text                       # code segment
.global _start              # _start is main()

_start:
   # calculate where my_msg is, save (push) 2 copies
   int $48						# get pid into eax
   
   movl %eax, %ebx				# store PID in ebx
   pushl %esp                   # ESP is end of dram
   popl %edx                    # use edx to manipulate
   
   subl $0x80DFF000, %edx		# virtual esp-2G+14M-4096 gives actual my_msg address
   movl $msg, (%edx)			# place msg into address my_msg address
   pushl %edx                   # save x on stack
   pushl %edx                   # save x to stack

   popl %eax    				# put translated msg address in eax
   movl %ebx, 0(%eax)			# set sender as this program's PID
   movl $5, 4(%eax)				# set msg.recipient = StdoutPID = 5 (4 bytes offset, .data)
   pushl %ebx					# store pid to stack
   int $54      				# call MsgSnd(&my_msg) interrupt
   popl %ebx					# retrieve pid from stack

   popl %eax					# put translated msg address in eax
   movl %ebx, 0(%eax)			# set sender as this program's PID
   int $55						# call MsgRcv(&my_msg)

   movl %eax, %edx				#copy recieved msg into edx
   movl 8(%edx), %eax			#copy OS_Clock to eax (offset 8, .data)
   int $58	 					# call Exit(my_msg.OS_clock as exit code)

.data      						# data segment always follows code segment in RAM
msg:     						# my_msg (this must match your own msg_t)
   .int 0      					# sender
   .int 0      					# recipient - 5 instructor allowed hard coding, attempting on-fly
   .int 0      					# OS_clock
   .ascii "Hello!\n\0" 			# string (length: 8)
   .rept 93    					# 8+93 = 101 characters
   .ascii "\0"  				# padding nulls  
   .endr        				# end repetition
   .int 0      					# STATUS
   .int 0      					# Owner/FD
   .int 0      					# bytes

#			Memory
#|--------------------------|->64MB
#|				.			|
#|				.			|
#|--------------------------|		|	dram[i]	|
#|	|___________________|	|		|___________|-->ESP
#|	|		dram[n]		|	|		|	TF_ptr	|
#|	|		dram[.]		|	|		|-----------|-->ESP-sizeof(TF_t)
#|	|		dram[.]		|	|		|	Owner	|
#|	|		dram[.]		|	|		|-----------|
#|	|		dram[i]		|	|		|	Addr	|
#|	|		dram[.]		|	|		|	(my_msg)|
#|	|		dram[1]		|	|------>|___________|->ESP-0x1000 (4096)---------------->0x80000000+x (virtual space) (14M+x in actual space)
#|	|		dram[0]		|	|														|	
#|	|-------------------|	|														|
#|	|		header		|	|														x
#|	|		0x80 (128b)	|	|														|
#|	|___________________|	|------------------------------------------------------->0x80000000 (2G) (virtual space) (2G-14M is actual space)
#|--------------------------|->14M													|
#|			MyOS.dli		|														|
#|--------------------------|->1M													|
#|				.			|														|
#|				.			|														|
#|				.			|														|
#|				.			|														|
#|--------------------------|->0b													|
#End of DRAM page (in virtual): ESP
#Beginning of DRAM page/my_msg (in virtual): ESP-4096
#X (in virtual): (ESP-4096)
#X (actual): (ESP-4096)-2G
#Beginning of DRAM (actual): 14M
#Location of my_msg (actual): 14M+x = 14M+(ESP-4096)-2G
#my_msg (actual): ESP-2G-4096+14M
#my_msg (actual): ESP-0x80000000-0x1000+0xe00000
#my_msg (actual): ESP-80DFF000
#my_msg (virtual) will be over 2G, so ESP-80DFF000 will be a positive number representing the address of my_msg actual



