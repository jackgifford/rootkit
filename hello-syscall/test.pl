#!/usr/bin/perl

$str = "Hello, Kernel!";
syscall(210, $str);
