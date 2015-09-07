#!/usr/bin/perl

$new_path = $ARGV[0];
$dir = $ARGV[1];
open f, "ls $dir/*.prl|";
while (<f>) {
  chomp;
  $file = $_;
  open g, $file;
  open h, ">$dir/tmp1";
  $a = <g>;
  print h "#!$new_path\n";
  while(<g>) {
    print h;
  }
  close g;
  close h;
  system "cp $file $file.old";
  system "mv $dir/tmp1 $file";
}
close f;
