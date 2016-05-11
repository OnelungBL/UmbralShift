# take5.s
# below is only a demo code, it calls Sleep(5) and Exit(99)
# change it to:
#    sleep for the period of seconds that is its PID, and
#    exit with its PID multiplied by two.
#
# Enter "makeit.pl take5.s" and it will automagically does:
#    as --32 take5.s -o take5.o 
#    link386 -nostartfiles -userapp take5.o -o take5
#    extract/convert executable take5 into text take5.bin
#
# The final generated file "take5.bin" is the purpose here, and
# it is to be included into file services (see Procedure.txt).
#
# If curious to see the memory map, enter: "as -a take5.s"

.text               # code segment 
.global _start      # _start is like main()

_start:             # instructions begin
    int $48         # get PID Interrupt  places PID in EAX
    int $49         # call sleep service, sleep for pid length

   movl $_start, %eax # use address of code segment into exit code

   int  $58         # call exit interrupt

.data               # how to define data (examples)
A:
   .long 100        # declare an integer, set it 100
msg1:
   .ascii "this is a string.\n\0" # declare a null-terminated string
msg2:                     # another string, 100 bytes/chars in total
   .ascii "Hello!\n"
   .rept 93               # repeat: 93 times
      .ascii "\0"         # filled with 93 NUL
   .endr                  # end of repeat


