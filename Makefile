# a little exercise

VERSION = 1.0

CC ?= gcc
CXX ?= g++

# Common compiler flags. See <http://blog.httrack.com/blog/2014/03/09/what-are-your-gcc-flags/> for
# a glance at most of them. Some flags are there for fancy reasons (such as -grecord-gcc-switches), others
# for security reasons (-fstack-protector-strong), and most ones for code correctness reasons.
# See also <https://wiki.debian.org/Hardening> for hardening hints
# We also use -march=native, as this is for this exercise, and not meant for production use.
# -Werror removed due to potentially unknown false positives (such as GCC 7.x related)
COMMON_CFLAGS := \
	-D_FORTIFY_SOURCE=2 \
	-D_DEFAULT_SOURCE \
	-march=native \
	-pipe \
	-O3 \
	-g3 \
	-fPIC \
	-fno-common \
	-fstack-protector \
	-fstack-protector-strong \
	-fvisibility=hidden \
	-pthread \
	-grecord-gcc-switches \
	-Wall \
	-Wextra \
	-Wbool-compare \
	-Wcast-align \
	-Wformat=2 \
	-Wformat-nonliteral \
	-Wformat-security \
	-Winit-self \
	-Wlogical-op \
	-Wmissing-format-attribute  \
	-Wno-unused-parameter \
	-Wpointer-arith  \
	-Wtrampolines \
	-Wundef \
	-Wuninitialized \
	-Wunused \
	-Wwrite-strings

# Common linker flags
COMMON_LDFLAGS := \
	-rdynamic \
	-Wl,-O1 \
	-Wl,--no-undefined \
	-Wl,--build-id=sha1 \
	-Wl,-z,relro \
	-Wl,-z,now \
	-Wl,-z,noexecstack

# Aimed for C (executable); -Wc++-compat is an attempt to relieve incurable C flaws
# (unused in this project)
CFLAGS ?= \
	-std=c99 \
	$(COMMON_CFLAGS) \
	-Wstrict-prototypes \
	-Wc++-compat

# Aimed for C++ only
CXXFLAGS ?= \
	-std=c++11 \
	-fno-exceptions \
	$(COMMON_CFLAGS)

# Aimed for link
LDFLAGS ?= $(COMMON_LDFLAGS)

EXECFLAGS ?=  \
	-fPIE

LIBS ?= -lstdc++

INSTALL = install
INSTALL_DATA ?= $(INSTALL) -m644
INSTALL_PROGRAM ?= $(INSTALL) -m755
MKDIR ?= mkdir -p -m 755
TAR ?= tar
GROFF ?= groff
BASH = bash
VALGRIND ?= valgrind --quiet --track-origins=yes --leak-check=full
DOT ?= dot

PREFIX ?= /usr
bindir ?= ${PREFIX}/bin/
man1dir ?= ${PREFIX}/share/man/man1
htmldir ?= ${PREFIX}/share/jitdemo/html
pdfdir ?= ${PREFIX}/share/jitdemo/pdf
man1ext ?= 1

.PHONY: default
default: build

.PHONY: all
all: build tests cleanobjs

.PHONY: clean
clean: cleanobjs
	rm -f *.so* *.dll jitdemo jitdemo.html jitdemo.pdf

.PHONY: cleanobjs
cleanobjs:
	rm -f *.o *.obj

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(EXECFLAGS)

%.so: %.o
	$(CC) -shared -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

%.svg: %.dot
	$(DOT) $< -Tsvg -o $@

.PHONY: build
build: jitdemo

jitdemo: jitdemo.o
	$(CC) -o $@ $^ -pie $(CXXFLAGS) $(LDFLAGS) $(EXECFLAGS) $(LIBS)

poc: poc.o
	$(CC) -o $@ $^ -pie $(CXXFLAGS) $(LDFLAGS) $(EXECFLAGS) $(LIBS)

sample: sample.so
	gdb --batch --quiet -ex "disass /r test" sample.so

.PHONY: tests
tests: build

.PHONY: valgrind
valgrind: build

.PHONY: graph
graph: classes.svg

.PHONY: man
man:
	$(GROFF) -man -Thtml jitdemo.1 > jitdemo.html
	$(GROFF) -man -Tps jitdemo.1 | ps2pdf - jitdemo.pdf

.PHONY: dist
dist:
	$(TAR) cfz jitdemo_$(VERSION).orig.tar.gz *.c *.h *.sh *.1 *.txt Makefile

.PHONY: install
install: build man
	$(MKDIR) $(DESTDIR)$(bindir)
	$(MKDIR) $(DESTDIR)$(man1dir)
	$(MKDIR) $(DESTDIR)$(htmldir)
	$(MKDIR) $(DESTDIR)$(pdfdir)
	$(INSTALL_PROGRAM) jitdemo $(DESTDIR)$(bindir)/
	$(INSTALL_DATA) jitdemo.1 $(DESTDIR)$(man1dir)/jitdemo.$(man1ext)
	$(INSTALL_DATA) jitdemo.html $(DESTDIR)$(htmldir)/
	$(INSTALL_DATA) jitdemo.pdf $(DESTDIR)$(pdfdir)/
