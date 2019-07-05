# Implementing a syscall 
Here's just a really simple example of how we can extend FreeBSD with custom system calls! 
Load this module, and then run the perl script to see how it works.

```
> make
> sudo kldload ./syscall.ko
> perl test.pl
"Hello, Kernel!"
```
