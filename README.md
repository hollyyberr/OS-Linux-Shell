# OS-Linux-Shell
Operating Systems Final Project

Developed by:
- Jake Brzyski
- Noah Paiva
- Holly Bernich
- Veronica Marquez
- Jacob Rappaport

----- PREREQUISITES -----

These steps are necessary in order to get the shell running.

- Open terminal and run 'sudo apt-get update' to update package repositories.

- In the terminal, run 'sudo apt-get install libreadline8 libreadline-dev' to install readline library for code compilation and usage.

----- OPTIONAL -----

We used Visual Studio Code inside of the Ubuntu virtual machine to edit our code as it is what we are familiar with.

- To install, open a terminal and run 'sudo snap install --classic code'

----- COMPILATION and EXECUTION -----

This step will compile the code to generate the shell file that can be run

 - Open a terminal in the same folder as 'main.c'

 - In the terminal, run 'gcc -o shell main.c -lreadline' (nothing will be printed if code compiles successfully)

 - In the terminal, run './shell' to launch the shell