wordsearch.debug: wordsearch.c
	gcc -Wall -Wextra -Wno-switch -pedantic -std=c11 -g -O0 -o wordsearch.debug wordsearch.c

wordsearch: wordsearch.c
	gcc -Wall -Wextra -Wno-switch -pedantic -std=c11 -g -O2 -o wordsearch wordsearch.c

all: wordsearch wordsearch.debug

clean:
	rm -f wordsearch wordsearch.debug
