#!/usr/bin/perl

sub catfile {
  open my $input, '<', $_[0] or die "can't open $_[0]: $!";
  while (<$input>) {
    #chomp;
    print $_;
  }
  close $input or die "can't close $file: $!";
}  

$num_args = $#ARGV + 1;
if ($num_args != 2) {
    print "\nUsage: makeinp.pl problem_def_name state_name\n";
    exit;
}

print "{\n";
print "\"problem\" : ";
catfile($ARGV[0]);
print ",\n";
print "\"state\" : ";
catfile($ARGV[1]);
print "\n}";
