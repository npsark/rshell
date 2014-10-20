#RSHELL

##Purpose
The goal of this project is to develop a simple shell into which the user can input commands.

At this point, rshell is capable of:
* executing most Unix commands.
* stringing commands together with the logical operators AND(&&) and OR(||).
* executing multiple commands separated by semicolons.

The hierarchy of the logical operators and semicolons is such that, the input is first separated by semicolons and then the resulting compound or simple commands are separated by && and ||.

For example, the command

`echo hello && echo how are you; echo test || echo echo isn't working`

is first parsed into the two compound commands

`echo hello && echo how are you`

and

`echo test || echo echo isn't working`

Next, each of the compound commands are parsed with respect to their corresponding logical operators

`echo hello` AND `echo how are you`

and

`echo test` OR `echo echo isn't working`

The final output would be

```
hello
how are you
test
```

##Building

To build rshell, first clone the repository to your local machine. Next, navigate to the folder that Makefile is in. Run the command make. This will create a bin folder in which the executable rshell will be located.
