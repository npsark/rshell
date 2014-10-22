RSHELL

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

##Building

To build *rshell*, first clone the repository to your local machine. Next, navigate to the folder that `Makefile` is in. Run the command `make`. This will create a `bin` folder in which the executable *rshell* will be located.


##Bugs
1. When text is being entered into *rshell*, it does not account for the quotes. As a result, something like `echo "Hello World"` will print, `"Hello World"` rather than the expected `Hello World`.
2. *rshell* cannot process commands when there are not spaces betweent the command and the parameters or between the parameters. However, it can process commands in which the parameters have been "pushed" together. For example, `top -i -c` is considered the same as `top -ic` while `top-i-c`, `top -i-c`, and `top-i -c` will all cause *rshell* to fail.
3. *rshell* does not recognize `Ctrl` commands. For example, in *bash* `Ctrl+c` will exit the current process immediately. *rshell* will not understand that command.
4. *rshell* does not process commands that have both && and || unless they are separated by semicolons. If it does come across this, it treats the && as ||. FOr example, `echo a || echo b && echo c` will be parsed as `echo a || echo b || echo c`.
5. *rshell* does not parse tabs correctly. For example, assuming 'T' represents a tab, `echo TTT"hello world"` would throw an error saying it can't find the directory.
