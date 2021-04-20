# OS-Linux-Shell
Operating Systems Final Project

PREREQUISITES

These steps are necessary in order to get the shell running.

- open terminal and run 'sudo apt-get update' to update package repositories.

- in the terminal, run 'sudo apt-get install libreadline8 libreadline-dev' to install readline library for code compilation and usage.

COMPILATION and EXECUTION

This step will compile the code to generate the shell file that can be run

 - open a terminal in the same folder as 'main.c'

 - in the terminal, run 'gcc -o shell main.c -lreadline' (nothing will be printed if code compiles successfully)

 - in the terminal, run './shell' to launch the shell