all:
	if [ ! -d bin ]; then mkdir bin; fi
	g++ src/ls.cpp -std=c++11 -o bin/ls -Wall -Werror -ansi -pedantic
	g++ src/cp.cpp -o bin/cp -Wall -Werror -ansi -pedantic
	g++ src/main.cpp -std=c++11 -o bin/rshell -Wall -Werror -ansi -pedantic

rshell:
	if [ ! -d bin ]; then mkdir bin; fi
	g++ src/main.cpp -std=c++11 -o bin/rshell -Wall -Werror -ansi -pedantic	

ls:
	if [ ! -d bin ]; then mkdir bin; fi
	g++ src/ls.cpp -std=c++11 -o bin/ls -Wall -Werror -ansi -pedantic

cp:
	if [ ! -d bin ]; then mkdir bin; fi
	g++ src/cp.cpp -o bin/cp -Wall -Werror -ansi -pedantic
