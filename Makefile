.PONEY: install

xsetkeylock: main.c
	gcc -oxsetkeylock main.c -W -Wall -lX11

install: xsetkeylock
	install xsetkeylock /usr/bin