# Motivation

А long time ago there was a library called libetc. It would load with LD_PRELOAD before any executable and replaced path argument for some of libc wrappers for syscalls. It hadn't been updated since 2008.

Lots of new system calls have been added to Linux kernel since then, and some of them even don't have libc wrapper (e.g. openat2). Old approach would require to painfully write new wrapper function for each new libc wrapper, without any possibility for some syscalls to be interrupted.

C. 2021 a new library appeared called zpoline. It allows to intercept all syscalls by using some dark magic.

It is only logical to port libetc functionality as zpoline syscall hook. As such, libzetc was born.

(for now libzetc is in development, so it is in the middle of birth process)

# How to build?

1. Clone zpoline repo & build zpoline
2. Run `make` in this repo

# How to use?

`LIBZPHOOK=/path/to/libzetc.so LD_PRELOAD=/path/to/libzpoline.so cmd`
