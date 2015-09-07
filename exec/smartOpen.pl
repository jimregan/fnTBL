#!/usr/local/bin/perl

# This is a perl module you can use by saying 
#
#     use lib '/usr/tools/lab/perl/';
#     require "smartOpen.pl";
#
# inside your perl program.
#
# then when you do
#
#     smartOpen(\*FILEHANDLE,$filename)
#
# instead of 
#
#     open(FILEHANDLE,$filename)
#
# inside your program it'll open the file for reading or writing and
# if it's compressed (ends with .gz), it'll uncompress it on the fly.
# It also supports the standard old "-" for using stdin/stdout.  Heck,
# you could have read the code below in the time you spent reading
# this. 
#
# It's not like it's so complex.
#
# -John
# jhndrsn@cs.jhu.edu
# 8/12/97


require 5.003;
use strict 'refs';
use strict 'vars';

sub smartOpen {
  my ($FILEHANDLE,
	  $filename) = @_;
  my $openstring;
  
  if ($filename =~ /\.gz$/
	  || $filename =~ /\.Z$/
	  || $filename =~ /\.bz2$/) {
	if ($filename =~ /^>/) {
	  $openstring = "|gzip -c -best $filename";
	} elsif($filename =~ /\.bz2/) {
	  $openstring = "bzcat -c $filename|";
	} else {
	  $openstring = "zcat -c $filename|";
	}
	return open($FILEHANDLE,$openstring);
  } 
  else {
	return open($FILEHANDLE,$filename);
  }
}


########################################
#  Don't delete that one below!
########################################

1;
