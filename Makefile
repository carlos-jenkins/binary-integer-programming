CC     = gcc -std=c99
DEBUG  = -Wall -g

CFLAGS = `pkg-config --cflags --libs glib-2.0` -lm
GFLAGS = `pkg-config --cflags --libs gtk+-3.0 gmodule-export-2.0` -lm

HEADRS = -Isrc/utils/
COMMON = src/utils/graphviz.c src/utils/latex.c src/utils/matrix.c src/utils/utils.c
GUI    = src/utils/dialogs.c

# Rules
all: clean bin/bip
test: clean bin/test/bip

# Main binary
bin/bip: src/bip/gui.c src/bip/format.c src/bip/bip.c src/bip/report.c
	$(CC) $(DEBUG) -o $@ $? $(HEADRS) $(COMMON) $(GUI) $(GFLAGS)

# Test binary
bin/test/bip: src/bip/test.c src/bip/bip.c src/bip/report.c
	$(CC) $(DEBUG) -o $@ $? $(HEADRS) $(COMMON) $(CFLAGS)

# Clean
clean:
	rm -f `find bin/ -executable -type f`
	rm -f `find bin/test/ -executable -type f`
	rm -f reports/*.tex
	rm -f reports/*.toc
	rm -f reports/*.aux
	rm -f reports/*.out
	rm -f reports/*.log
	rm -f reports/*.pdf
	rm -f reports/*.eps
	rm -f reports/*.png
