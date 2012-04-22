default: pmgui.o pmlib.o
	gcc -g -o radeon-pm-gui pmgui.o pmlib.o `pkg-config --libs gtk+-3.0`

pmgui.o: pmgui.c
	gcc `pkg-config --cflags gtk+-3.0` -c pmgui.c
	
pmlib.o: pmlib.c
	gcc `pkg-config --cflags gtk+-3.0` -c pmlib.c

clean:
	rm radeon-pm-gui pmgui.o pmlib.o || true
