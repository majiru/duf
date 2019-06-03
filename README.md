# Discount Userspace Fuzzer
A simple fuzzer for plan9 written to be used from userspace.

# Usage
`duf [-a] [-t numproc] cmd args...`

The a flag specifies to turn the random input into valid ascii before sending it to the child process.

The t flag specifies the number of child processes to spawn off.
