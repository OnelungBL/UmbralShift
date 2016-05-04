#!/usr/bin/perl
# makeit.pl
#
# Perl script to compile and convert executable to text
# input: xxx.s (assembly program code)
# output: xxx.bin (text to be included into the file system)
#
# Author: Weide Chang, Copyfree 2016

$#ARGV == 0 || die "Usage: makeit.pl xxx.s (e.g., makeit.pl abc.s)\n";

$ARGV[0] =~ s/\.s//;                        # get rid of .s part
`as --32 $ARGV[0].s -o $ARGV[0].o`;         # GNU assembloer
`link386 -nostartfiles -userapp $ARGV[0].o -o $ARGV[0]`;  # linker

$in_file = $ARGV[0];
$out_file = "$in_file.bin";

open IN, "< $in_file" || die "Can't read $in_file,";     # read buffer IN
open OUT, "> $out_file" || die "Can't write $out_file,"; # write buffer OUT

binmode IN;   # read in binary mode

$in_size = 0;
$bytes = "";
while (($n = read IN, $byte, 1) != 0) { # read a byte at a time till EOF
   $bytes .= $byte;                     # concatenate them up
   $in_size++;
}

$out_size = $in_size;     # keep header, or $in_size - 128; # skip header
print "$in_file is $in_size bytes, $out_file will contain $out_size pairs of hex digits.\n";

for $i (0 .. $in_size - 1) {
#   next if($i < 128); # skip header, 1st 128 bytes

   $byte = substr($bytes, $i, 1); # get a char in the string (byte stream)

   $str = sprintf(" 0x%x", ord($byte)); # convert output to hex format

   print OUT $str;

   print OUT "," unless($i == $in_size - 1); # separate numbers with , unless last

   if((($i + 1) % 8 == 0) || ($i == $in_size - 1)) {
      print OUT "\n"; # every 8 hex pairs or last number, add newline
   }
}

close OUT; # close output file handle

