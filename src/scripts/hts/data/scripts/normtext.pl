#!/usr/bin/perl
# ----------------------------------------------------------------- #
#           The HMM-Based Speech Synthesis System (HTS)             #
#           developed by HTS Working Group                          #
#           http://hts.sp.nitech.ac.jp/                             #
# ----------------------------------------------------------------- #
#                                                                   #
#  Copyright (c) 2012-2015  Nagoya Institute of Technology          #
#                           Department of Computer Science          #
#                                                                   #
# All rights reserved.                                              #
#                                                                   #
# Redistribution and use in source and binary forms, with or        #
# without modification, are permitted provided that the following   #
# conditions are met:                                               #
#                                                                   #
# - Redistributions of source code must retain the above copyright  #
#   notice, this list of conditions and the following disclaimer.   #
# - Redistributions in binary form must reproduce the above         #
#   copyright notice, this list of conditions and the following     #
#   disclaimer in the documentation and/or other materials provided #
#   with the distribution.                                          #
# - Neither the name of the HTS working group nor the names of its  #
#   contributors may be used to endorse or promote products derived #
#   from this software without specific prior written permission.   #
#                                                                   #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            #
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       #
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          #
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          #
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS #
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          #
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   #
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON #
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   #
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    #
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           #
# POSSIBILITY OF SUCH DAMAGE.                                       #
# ----------------------------------------------------------------- #

if ( @ARGV != 1 ) {
   print "txtnorm.pl infile\n";
   exit(0);
}

$infile = $ARGV[0];

# load text
$text = "";
open( INPUT, "$infile" ) || die "Cannot open file: $infile";
while ( $input = <INPUT> ) {
   $text .= $input;
}
close(INPUT);

@word = ();
@type = ();
@left = ();

while ( $text =~ /(['0-9a-zA-Z']+)/ ) {    # digit or alphabet or apostrophe
   my $w         = $1;
   my $index     = index( $text, $w );
   my $separator = substr( $text, 0, $index );
   my $t         = "unknown";
   my $l         = "unknown";

   if ( $w =~ /(^[a-zA-Z]+$)/ ) {
      $t = "alphabet";
   }
   elsif ( $w =~ /(^[0-9]+$)/ ) {
      $t = "digit";
   }

   if ( $separator eq "-" ) {
      $l = "hyphen";
   }
   else {
      $separator =~ s/\s//g;
      if ( $separator eq "." ) {
         $l = "period";
      }
      elsif ( $separator eq "" ) {
         $l = "space";
      }
      elsif ( $separator eq "," ) {
         $l = "comma";
      }
      elsif ( $separator eq "?" ) {
         $l = "question";
      }
      else {
         print STDERR "WARNING: unknown separator [$separator]\n";
         if ( index( $separator, "?" ) >= 0 ) {
            $l = "question";
         }
      }
   }

   push( @word, $w );
   push( @type, $t );
   push( @left, $l );

   substr( $text, 0, $index + length($w) ) = "";
}

$question = 0;
if ( index( $text, "?" ) >= 0 ) {
   $question = 1;
}

if ( @word < 1 ) {
   exit(0);
}

for ( $i = 0 ; $i < @word ; $i++ ) {
   if ( $i == 0 ) {
      print "$word[$i]";
   }
   else {
      if ( $type[ $i - 1 ] eq "digit" && $type[$i] eq "digit" && $left[$i] eq "period" ) {
         print ".$word[$i]";
      }
      elsif ( $left[$i] eq "hyphen" ) {
         print "-$word[$i]";
      }
      elsif ( $left[$i] eq "space" ) {
         print " $word[$i]";
      }
      else {
         print ", $word[$i]";
      }
   }
}
if ($question) {
   print "?\n";
}
else {
   print ".\n";
}
