# This is used for profiling...
#CC=cc -pg
# This is the normal one
CC=cc
CFLAGS=-g -O2
.SUFFIXES : .c .o

melquery: melquery.o  meldb.o melsim.o general.o prefile.o melstrings.o
	$(CC) -o  melquery melquery.o meldb.o melsim.o prefile.o general.o melstrings.o -lm


meldb.o: meldb.c meldb.h Makefile melodies.h melsim.h melodies.h prefile.h melstrings.h general.h 

melquery.o: melquery.c melquery.h prefile.h general.h melodies.h melstrings.h melstrings.c Makefile

prefile.o: prefile.c prefile.h melodies.h

melodies.o:   Makefile


melsim.o: melsim.c Makefile melsim.h melodies.h

melstrings.o: melstrings.c melstrings.h Makefile

outputroots: outputroots.o general.o melstrings.o
	$(CC) -o  outputroots outputroots.o melstrings.o  general.o 

outputroots.o: outputroots.c

general.o: general.c general.h Makefile
	$(CC) -c general.c

clean: 
	rm *.o

#generic rules

.c.o :
	 $(CC) $(CFLAGS) -I .. -c $< 






