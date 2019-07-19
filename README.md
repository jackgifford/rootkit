# rootkit

This is the repo for the rootkits I develop for my something awesome project for COMP6841. 
They run on FreeBSD 12 and require you to have the source code available. Like everything only install these modules on systems you have permission to use! Ideally run these in a virtual machine, or in an environment where there's no consequences if everything fails.

Everything I've worked on here is strictly for educational purposes. 

* esce -> Privelige escelation, provide pid, and uid will be changed to 0.
* file-hiding -> hide files, add your files in by absolute path, before compiling enables reboot persistence.
* keylog -> writes all user input to a file. Consider hiding this file.
* proc-hiding -> Hides a proc from calls like `top`.
* remote-shell -> the most fun, sends a reverse shell to your machine. Select your ip before compiling. Consider hiding ports too!

This could be easier to use, I've made it at least somewhat difficult to get up and running, this is by design!
