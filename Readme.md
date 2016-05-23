# Minishell

This is a simple shell for linux that executes intern and exetern commands in foreground and background. 
## Compilation
Just use the makefile as:

```sh
$ make
```
This will compile the project and create the executable. It is always highly recommendable to clean all the *.o and executable files. To do  this, simply do: 
```sh
$ make clean
```
If an error appears when you do the make, that must be the readline library. Make sure you have it. To ensure, type: 
```sh
$ apt-cache search readline
```
If the library is already installed, you will see a list of packages. Two of those have to be libreadline and libreadline-dev. If you dont have it, simply write: 
```sh
$ apt-get install libreadline6 libreadline6-dev
```
If it still not working, do this:
```sh
$ gcc -o shell shell.c -L/usr/local/lib -I/usr/local/include -lreadline
```
### Execution
Launch the exetuable: 
```sh
$ ./shell
```
Now you are running the minishell. 

List of funtionalities:
* Inter and extern command interpreter
* Extern command in foreground and background. 
* Output redirection
* Use up and down arrow

