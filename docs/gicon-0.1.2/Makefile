INSTALL = /opt/gicon

CC      = gcc
CFLAGS  = -g -Wall `gnome-config --cflags gnome gnomeui libglade`
LDFLAGS = `gnome-config --libs gnome gnomeui libglade`

LIBS    = -L/usr/lib/

OBJS    =  util.o tty.o nkn.o file.o print.o \
           gui.preferences.o gui.controls.o gui.settings.o \
           gui.main.o gui.notebook.o gui.print.o gui.memoholder.o

PROGRAM = gicon

all:	$(OBJS)
	gcc $(LDFLAGS) $(OBJS) $(LIBS) -g -lm -o $(PROGRAM)

clean:
	rm -f *.o *~ core
tags:
	cd $(HOME)Projects/gicon-0.1.2; find . | xargs etags -o TAGS
id:
	cd $(HOME)/Projects/gicon-0.1.2; mkid -m $(HOME)/share/id-lang.map -o ID *

install:
	mkdir $(INSTALL); \
	cp -R help $(INSTALL); \
	cp -R pixmaps $(INSTALL); \
	cp gicon gui.xml $(INSTALL);

uninstall:
	rm -rf $(INSTALL); \
	rm $(HOME)/.gicon
