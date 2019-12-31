# sub Makefile for GAME folder

%.d : %.cpp
	set -e; $(CXX) -MM $(CFLAGS) $< \
		| sed 's^\($*\)\.o[ :]*^\1.o $@ : ^g' > $@; \
		[ -s $@ ] || rm -f $@

OBJS = game.o singe.o

.SUFFIXES:	.cpp

all:	${OBJS}

include $(OBJS:.o=.d)

.cpp.o:
	${CXX} ${CFLAGS} -c $< -o $@

clean:
	rm ${OBJS} *.d

