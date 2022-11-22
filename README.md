# SHELL SIMULATOR (SHELLSIM)
Nov 2022

How to use: 
1. run ./main
2. type commands like in terminal
3. to stop shellsim type ctrl+D

Completed functions:
 * execution of simple commands, e.g. `gcc -o hi helloworld.c`
 * conveyor: `./out | ./in -a | ./inc -f 789`
 * input/output redirection with `>`, `<`, `>>`
 * consistent execution with `;`
 * --one-str option
 * background execution: `./prog &`
 * conditional statements `&&`, `||`
 * brackets `()`
 

Project structure:
1. main.c is the main shellsim file, which handles users input, parses string to commands and executes them in the correct order
2. command_list.c
3. types.c and derectives.c
