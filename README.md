#RSHELL

##Purpose
The goal of this project is to develop a simple shell into which the user can input commands.

##Function
At this point, rshell is capable of:
* executing most Unix commands.
* stringing commands together with the logical operators AND(&&) and OR(||).
* executing multiple commands separated by semicolons.
* processing comments after the pound sign(#).
* exiting when `exit` is entered at the prompt.
* displaying the user's login and host name at the prompt.
* piping the out put of a command into the input of another.
* redirecting the input and output of a command from and to any file.
* ignoring the interrupt signal unless a child process is running.
* changing the current working directory by means of the `cd` command. Only one path is considered among the arguments. If more than one is given, only the first is used.
* manually searching for programs in each path listed in the PATH environment variable.

###Logic Operators

The hierarchy of the logical operators and semicolons is such that, the input is first separated by semicolons and then the resulting compound or simple commands are separated by && and ||.

For example, the command

`echo A && echo B; echo C || echo D`

is first parsed into the two compound commands

`echo A && echo B`

and

`echo C || echo D`

Next, each of the compound commands are parsed with respect to their corresponding logical operators

`echo A` AND `echo B`

and

`echo C` OR `echo D`

The final output would be

```
A
B
C
```

At this point, *rshell* checks to see if there is an OR operator(||) in the input. If there is, it processes the command such that, it stops executing once it succesfully executes one of the comands separated by the OR operator. If there is no OR operator, it processes the input such that, it only executes the next command if the last one was successful.

Of course, this is only relevent for compound commands. Single commands simply execute and succeed or fail.

###Piping

Piping is done by inserting the '|' character in between two commands. For example:

`cat Makefile | grep g++`

The output of `cat`, which is the contents of `Makefile` is passed to `grep` as input. `grep` then searches the input for `g++` and outputs its results to the screen.

Pipes can be chained indefinitely. `cat Makefile | grep g++ | grep main.cpp` executes `cat`, passes the output to `grep`, which in turn passes its output to the next `grep`, which finally outputs to `stdout`.

###Input & Output Redirection

IO redirection is done using the '<', '>', and '>>' symbols.

`cat < Makefile` tells `cat` to get its input from `Makefile`.
`cat Makefile > Makefile_cpy` tells `cat` to send it output to `Makefile_cpy`.
`cat README.md >> Makefile_cpy` tells `cat` to append its output to `Makefile_cpy`.

These can be mixed and matched in reasonable ways. Specifically, '>' and '>>' should not appear in the same command. `cat Makefile >file1 >>file2` would cause an error because *rshell* does not know where to send the output. However, '>' and '>>' can always appear in a command that contains '<' because rshell knows that one is redirecting output while the other is redirecting input.

The location of the input and output redirections in a command relative to the other arguments of the command does not matter.
`grep <Makefile -o g++` is the same as `grep g++ -o <Makefile`.

##Building

Enter the following commands into *bash*:

* git clone https://github.com/npsark/rshell.git

* cd rshell

* make rshell

After entering these commands, the *rshell* program will be stored under the `bin` directory.

##Valgrind test
```
nat@Nat-Debian:~/cs100/rshell$ valgrind bin/rshell
==4241== Memcheck, a memory error detector
==4241== Copyright (C) 2002-2011, and GNU GPL'd, by Julian Seward et al.
==4241== Using Valgrind-3.7.0 and LibVEX; rerun with -h for copyright info
==4241== Command: bin/rshell
==4241== 
nat@Nat-Debian:/home/nat/cs100/rshell$ ls
bin  hw4.md  LICENSE  Makefile	README.md  src	tests
nat@Nat-Debian:/home/nat/cs100/rshell$ cd /home
nat@Nat-Debian:/home$ ls
nat
nat@Nat-Debian:/home$ exit
==4241== 
==4241== HEAP SUMMARY:
==4241==     in use at exit: 410 bytes in 18 blocks
==4241==   total heap usage: 201 allocs, 183 frees, 13,152 bytes allocated
==4241== 
==4241== LEAK SUMMARY:
==4241==    definitely lost: 0 bytes in 0 blocks
==4241==    indirectly lost: 0 bytes in 0 blocks
==4241==      possibly lost: 230 bytes in 8 blocks
==4241==    still reachable: 180 bytes in 10 blocks
==4241==         suppressed: 0 bytes in 0 blocks
==4241== Rerun with --leak-check=full to see details of leaked memory
==4241== 
==4241== For counts of detected and suppressed errors, rerun with: -v
==4241== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 4 from 4)
```
##CPPCheck
```
nat@Nat-Debian:~/cs100/rshell$ cppcheck src/main.cpp 
Checking src/main.cpp...
```
##Bugs
1. When text is being entered into *rshell*, it does not account for the quotes. As a result, something like `echo "Hello World"` will print, `"Hello World"` rather than the expected `Hello World`.
2. *rshell* cannot process commands when there are not spaces betweent the command and the parameters or between the parameters. However, it can process commands in which the parameters have been "pushed" together. For example, `top -i -c` is considered the same as `top -ic` while `top-i-c`, `top -i-c`, and `top-i -c` will all cause *rshell* to fail.
3. *rshell* does not recognize `Ctrl` commands. For example, in *bash* `Ctrl+c` will exit the current process immediately. *rshell* will not understand that command.
4. *rshell* does not process commands that have both && and || unless they are separated by semicolons. If it does come across this, it treats the && as ||. For example, `echo a || echo b && echo c` will be parsed as `echo a || echo b || echo c`.
5. *rshell* does not parse tabs correctly. For example, assuming 'T' represents a tab, `echo TTT"hello world"` would throw an error saying it can't find the directory.
6. If *rshell* encounters more than one output redirection symbol within a single command, it chrashes. If the command is a compound command separated by semicolons, any of the commands between semicolons can have output redirection regardless of whether the others do.
7. If a command redirects its output to a file and then attempts to pipe its output to another command, *rshell* will crash (ex. `cat Makefile >file | grep g++`).
8. If a program is running within *rshell* and ^c is enterred, it does not kill the child process. Instead, it send the interrup signal. As a result, if the child process handles interrupt signals unusually, the child's handling is executed.


#LS

##Purpose
The goal of this program is to replicate the standard *ls* program with the parameters `-a`, `-l`, and `-R`.

##Function

At this point, *ls* is capable of:
* handling zero or more file paths.
* handling file paths that represent directories as well ad regular files.
* handling the `-a` parameter.
* handling the `-l` parameter.
* handling the `-R` parameter.
* parsing `-alR`, `-a -l -R`, `-al -R`, etc. as the same set of parameters.

As far as arguments are concerned, *ls* is fairly robust. Any number of arguments can be passed with any number of spaces in between any of them. The arguments can also be placed anywhere amongst the file paths. Furthermore, any combination of the three parameters can be passed with any number of them separate or combined (`-alR` = `-a -lR` = `-al -R` = `-lRa` = `-Ral` = etc.).

When a path is passed, ls checks if it starts with the '/' character. If it does, it assumed it is an absolute path. If it does not start with the '/' character, it assumes it is a relative path. For example, `/home/nat/rshell/bin` is the same as `bin`. However, `/bin` is not the same as `bin`. Also, any '/' characters at the end of the path are ignored (i.e. `bin/` = `bin`).

##Building

Enter the following commands into *bash*:

* git clone https://github.com/npsark/rshell.git

* cd rshell

* make ls

After entering these commands, the *ls* program will be stored under the `bin` directory.

##Valgrind Test
```
root@Nat-Debian:/home/nat/cs100/rshell# valgrind bin/ls 
==6134== Memcheck, a memory error detector
==6134== Copyright (C) 2002-2011, and GNU GPL'd, by Julian Seward et al.
==6134== Using Valgrind-3.7.0 and LibVEX; rerun with -h for copyright info
==6134== Command: bin/ls
==6134== 
bin  dir  LICENSE  Makefile  README.md  README.md~  src  tests  

==6134== 
==6134== HEAP SUMMARY:
==6134==     in use at exit: 0 bytes in 0 blocks
==6134==   total heap usage: 225 allocs, 225 frees, 42,802 bytes allocated
==6134== 
==6134== All heap blocks were freed -- no leaks are possible
==6134== 
==6134== For counts of detected and suppressed errors, rerun with: -v
==6134== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 4 from 4)
```

##CPPCheck Test
```
nat@Nat-Debian:~/cs100/rshell$ cppcheck src/ls.cpp 
Checking src/ls.cpp...
nat@Nat-Debian:~/cs100/rshell$
```
##Bugs
1. An extra line is printed after the main output is diplayed.
2. The total number of blocks that is printed at the top of the output witht the `-l` parameter is sometimes off. It seems to typically be smaller than it should be.
3. There is a slight difference in how the list of files to be printed are alphabetized. For example, the standard `ls` prints the first four files of the *rshell* directory as `.  ..  bin  dir  .git` while this version of ls prints `.  ..  .git  bin  dir`.
4. The standard `ls` prints different file types in different colors. This version simply prints every type of file in the same color.


#CP

##Purpose
The goal of this program is to replicate the standard *cp* program with two path parameters. It also features an option that allows the user to run three versions of the copy algorithm and displays how long each one took. This is controlled by a third, optional parameter.

##Function

At this point, *cp* is capable of:
* handling any number of arguments.
* handling an optional argument to determine if all three copy methods are executed.
* handling nonexistent input paths.
* handling input paths that point to directories.

If the user enters more than two paths or less than 1 path, cp throws an error. If the input path is a directory, or does not exist, an error is thrown. If the output path already exists, an error is thrown. The optional parameter must be a '-' character followed by anything other than '0' in order for the full report to be printed.

##Building

Enter the following commands into *bash*:

* git clone https://github.com/npsark/rshell.git

* cd rshell

* make cp

After entering these commands, the *cp* program will be stored under the `bin` directory.

##Valgrind Test
```
nat@Nat-Debian:~/cs100/rshell$ valgrind cp testFile cpFile
==6925== Memcheck, a memory error detector
==6925== Copyright (C) 2002-2011, and GNU GPL'd, by Julian Seward et al.
==6925== Using Valgrind-3.7.0 and LibVEX; rerun with -h for copyright info
==6925== Command: cp testFile cpFile
==6925== 
==6925== 
==6925== HEAP SUMMARY:
==6925==     in use at exit: 0 bytes in 0 blocks
==6925==   total heap usage: 35 allocs, 35 frees, 43,416 bytes allocated
==6925== 
==6925== All heap blocks were freed -- no leaks are possible
==6925== 
==6925== For counts of detected and suppressed errors, rerun with: -v
==6925== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 4 from 4)
```

##CPPCheck Test
```
nat@Nat-Debian:~/cs100/rshell$ cppcheck src/cp.cpp 
Checking src/cp.cpp...
nat@Nat-Debian:~/cs100/rshell$
```

##Bugs
1. Cannot copy directories (no recursive copy functionality).
