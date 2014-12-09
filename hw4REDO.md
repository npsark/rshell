#Output Formatting in C++
Almost anyone who has spent more than a few minutes using C++ has heard of, if not used cout, cin, and cerr. However, many C++ programmers are unaware of some of the obscure tricks and techniques that are available in the *iostream* library. In this tutorial we are going to explore some of those forgotten bits as well as some other formatting features and come out with a few new tools that we can use to format text in more interesting ways.

##iomanip
*iomanip* is a library dedicated to somewhat miscellaneous input and output manipulations. Some of these are incredibly usefull, while othere seem pretty random. Let's get into some of them.

###setbase()
`setbase()` is a function that allows the user to pick what number system is used to ouput integers. Specifically, it can be set to decimal, octal, or hexadecimal. Here's an example:
```
cout << setbase(16) << 26 << endl;
cout << setbase(10) << 26 << endl;
cout << setbase(8) << 26 << endl;

//output:
//1a
//26
//32
```
The above code will output the number 26 three times. Once in hexadecimal, once in decimal, and once in octal. This is a really simple function that might not be screaming usefullness, but does come in handy every now and then.

###setw()
`setw()` is a really usefull fuction when dealing with columns of output. In terms of a spreadsheet, `setw()` essentially sets the width of a cell. Here's an example:
```
cout << setw(10) << "apple";
cout << setw(10) << "orange";
cout << setw(10) << "cherry" << endl;
cout << setw(10) << "plum";
cout << setw(10) << "pumpkin";
cout << setw(10) << "grape" << endl;

//output:
//     apple    orange    cherry
//      plum   pumpkin     grape
```
The above code prints out a table of six elements into two rows and three columns. Each element can be up to 10 characters long. If any element is longer than 10 characters, it will push the next elements on the same row over. `setw()` does not permenently set the new width. It is only set for the next bit of text being printed. That is why it is called before every line of output in the given code.

There are a couple more things you can do with `setw()`. First of all, you can set the justification. This is also wonderfully simple. Here's how you would left justify the last example:
```
cout << left;
cout << setw(10) << "apple";
cout << setw(10) << "orange";
cout << setw(10) << "cherry" << endl;
cout << setw(10) << "plum";
cout << setw(10) << "pumpkin";
cout << setw(10) << "grape" << endl;

//output:
//apple     orange    cherry    
//plum      pumpkin   grape     
```
All I had to do was add `cout << left;` before the text I wanted to justify. Unlike, `setw()` the justfication only has to be set once and all of the following output will adhere to it until, it is set to something else. The three justification options are `left`, `right`, and `internal`. The first two are pretty obvious. `internal`, on the other hand is pretty strange. What it does is left justify the signs of numbers and right justify the magnitude of number. For example, `cout setw(5) << internal << -57` could output `-  57`. According to the documentation, this is aparently usefull for accounting documents. So here's to you, accountants out there!

The last thing I want to discuss regarding `setw()` is `setfill()`. `setfill()` allows you to set what character is used in the empty spaces that often appear when using `setw()`. Example:
```
cout << left << setfill('.');
cout << setw(10) << "apple";
cout << setw(10) << "orange";
cout << setw(10) << "cherry" << endl;
cout << setw(10) << "plum";
cout << setw(10) << "pumpkin";
cout << setw(10) << "grape" << endl;

//output:
//apple.....orange....cherry....
//plum......pumpkin...grape.....
```
Pretty straight forward again, just pick a character and that's what `setw()` will fill into the empty spaces.

*iomanip* has a few more interesting functions that I'm just going to skip for now. Feel free to look them up in the C++ documentation for [*iomanip*](http://www.cplusplus.com/reference/iomanip/).

##ANSI
Now I'm going to get into using ANSI codes to further format our output. After this, you will be able to set the foreground and background colors of, underline, and bold your output.

###Colors
Using ANSI codes in C++ is pretty funky so let's start with an example.
```
cout << "\033[41;33mHello World!" << endl";
```
Inserting an ANSI code to change text color start with inserting `\033[` before the text you want to ouput. So for the example above, it was inserted between the opening quotation mark and the 'H' of "Hello World!". This signifies the beginning of the ANSI code. Next come some numbers separated by semicolons. The numbers are what determine which format feature you want to use. For example, anything between 30 and 37 sets the foreground color. Anything between 40 and 47 sets the foreground color. To see exactly which colors each number corresponds to, check out this [table](http://ascii-table.com/ansi-escape-sequences.php). The last bit you need to add to complete the ANSI code is the character 'm'. This signifies the end of the code.

The output of the code above will be the text, "Hello World!" highlighted in red and printed in yellow. Checking the ANSI table I mentioned, we can see that 41 corresponds to red highlighting and 33 corresponds to yellow text.

###Misc Formatting
Some other attributes you can set with ANSI codes include underlining and bolding. These are just as simple to set as the colors are. The number 4 corresponds to underlining and 1 corresponds to bolding. So, to add these to colored and highlighted text, we simply add 1 and 4 to the list of numbers in the ANSI code. Here's what that will look like:
```
cout << "\033[41;33;1;4mHello World!" << endl";
```

So thats a small taste of what you can do with ANSI. If you followed this tutorial and cannot get the ANSI formatting to work, it is possible you are using a console that does not support those features. I am using the default terminal that comes with Debian and everything works fine. The console that comes with Code::Blocks on Windows however, does not support ANSI. A common symptom of a console that does not support ANSI is junk characters being printed.
