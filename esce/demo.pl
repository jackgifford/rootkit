$pid = shift @ARGV;
syscall(210, int($pid));

