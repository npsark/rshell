all:
	if [ ! -d bin ]; then mkdir bin; fi
	g++ src/main.cpp -o bin/rshell -Wall -Werror -ansi -pedantic

rshell:
	if [ ! -d bin ]; then mkdir bin; fi
	g++ src/main.cpp -o bin/rshell -Wall -Werror -ansi -pedantic	
