/* File: melquery.c
   Author: Sandra Uitdenbogerd
   Date: 16 Jun 00
   This program compares a melody query against a list of melodies
   "preprocessmidimels.tcl" which stores each interval as:
   pitch difference + 127, with 255 reserved for rests
   rhythm contour (L=longer, S=shorter, R=remain the same)
   stress contour (L=louder, S=softer, R=remain the same)
   Rests don't have a stress contour byte

   The user chooses the similarity measure and type of comparison
   by way of command line arguments.
*/
#define debug 0
#define DEBUG 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/limits.h>

/*#include "top.h" */
#include "melodies.h" 
#include "melsim.h"
#include "melquery.h"
#include "meldb.h"
#include "prefile.h"
#include "melstrings.h"

#include "general.h"

char *version = "2006 MIREX version";

int main( int argc, char *argv[] )
{
int i, argno, numReturned=1000, maxQueryLength = INT_MAX, ret, root;
int mismatch = -1, indel = -2;
long measure, position1, position2;
char *absolutestr = NULL;
char *str1, *str2, *queryfilename = NULL, *melfilename, c,
  *database = NULL,  *compresstype = strdup( "n" );
 char *querystring;
FLAGS myflags;
SONG *queryPtr;
RESULTLIST *msrsPtr;
FILE *dbPtr;
struct stat *dbfileDetails;
int matchset = 0, smallmatchset = 0;

/* External variables */
 extern char *optarg; /* points to an option argument */
 extern int optind; /* index to the next argv argument */
 extern int optopt; /* last known option char returned by getopt */
 extern int opterr;
 extern int optreset;
 extern int errno;

#if DEBUG
 printf("Melody comparison program\n\n" );
 printf( "compresstype has been initialised to %s\n", compresstype );
#endif

 initFlags( &myflags );
/* Process command line arguments */
 while((c = getopt(argc, argv, 
		  "A:q:t:n:d:p:g:c:m:M:l:i:a:w:W:D:k:o:bresSfhvINQ")) != EOF)
   {
     switch(c)
       {
	 /*************************** Args ***********************/
       case 'a':/* absolute or normalisation factor */
	 if( *optarg == 'l' )
	   myflags.normlog = 1;
	 else
	   {
	     ret = sscanf( optarg, "%d", &root );
	     myflags.normroot = root;
	     myflags.normlog = 0;
	     if( ret != 1 )
	       doError( "Invalid argument for absolute/normalisation\n" );
	   }
#if DEBUG
	 displayFlags( myflags );
	 printf( "Normalisation factor is %d and %d\n", myflags.normlog,
		  myflags.normroot );
#endif
	 break;
       case 'b':/* traceback */
	 myflags.traceback = 1;
#if DEBUG
	 printf( "Trace back to find match location\n" );
#endif
	 break;
       case 'c':/* compress answers type */
	 compresstype = strdup(optarg);
#if DEBUG
	 printf( "Compress answers type is %s\n", compresstype );
#endif
	 break;
       case 'd':/* Melody database - a file containing a list of files */
	 database = strdup(optarg);
	 myflags.database = database;
#if DEBUG
	 printf( "Database is %s \n", database );
#endif
	 break;
       case 'e':/* exhaustive search */
#if DEBUG
	 printf( "Exhaustive search\n" );
#endif
	 break;
       case 'f':/* fast */
	 myflags.fast = 1;
#if DEBUG
	 printf( "Fast run using a text database file\n" );
#endif
	 break;
       case 'g':/* size of n-gram used */
	 ret = sscanf( optarg, "%d", &myflags.ngramsize );
	 if( ret != 1 || myflags.ngramsize > 8 || myflags.ngramsize < 1)
	   doError( "Invalid argument for n-gram size, should be an in in the range 1 to 8\n" );
#if DEBUG
	 printf( "N-gram size is now %d\n", myflags.ngramsize );
#endif
	 break;
       case 'h':/* help */
	 showHelp();
	 exit(0);
	 break;
       case 'i':/* indel value */
	 ret = sscanf( optarg, "%d", &indel );
	 if( ret != 1 )
	   doError( "Invalid argument for quantity returned\n" );
	 indel = -indel;
	 myflags.indel = indel;
#if DEBUG
	 printf( "indel value is now %d\n", indel );
#endif
	 break;
       case 'k':/* max number of skips in adjusted interval search */
	 ret = sscanf( optarg, "%d", &myflags.maxskips );
	 if( ret != 1 || myflags.maxskips <= -1 )
	   doError( "Invalid argument for max skips\n" );
#if DEBUG
	 printf( "Maximum skips is now %d\n", myflags.maxskips );
#endif
	 break;
       case 'l':/*max length of query to use */
	 ret = sscanf( optarg, "%d", &maxQueryLength );
	 if( ret != 1 )
	   doError( "Invalid argument for max query length\n" );
#if DEBUG
	 printf( "Max query length is now %d\n", maxQueryLength );
#endif	
	 break;
       case 'm':/* mismatch value */
	 ret = sscanf( optarg, "%d", &mismatch );
	 if( ret != 1 )
	   doError( "Invalid argument for quantity returned\n" );
	 mismatch = -mismatch;
	 myflags.mismatch = mismatch;
#if DEBUG
	 printf( "Mismatch value is now %d\n", mismatch );
#endif
	 break;
       case 'n':/* note comparison type, eg. intervals, contour */
	 myflags.notetype = *optarg;
#if DEBUG
	 printf( "Note type is %c \n", myflags.notetype );
#endif
	 break;
       case 'o':/* output type, i=incipit track, k=karaoke track, h=humming */
	 myflags.outputtype = *optarg;
#if DEBUG
	 printf( "Output type is %c \n", myflags.outputtype );
#endif
	 break;
       case 'p':/* size of pool returned */
	 ret = sscanf( optarg, "%d", &numReturned );
	 if( ret != 1 )
	   doError( "Invalid argument for quantity returned\n" );
#if DEBUG
	 printf( "Number of returned files is now %d\n", numReturned );
#endif
	 break;
       case 'q':/* query file */
	 queryfilename = strdup(optarg);
#if DEBUG
	 printf( "Query file name is %s\n", queryfilename );
#endif
	 break;
       case 'r':/* rests */
	 myflags.rests = 1;
#if DEBUG
	 printf( "Rests are included\n" );
#endif
	 break;
       case 's':/* stress */
#if DEBUG
	 printf( "Stress is included\n" );
#endif
	 break;
       case 't':/* measure type */
	 myflags.measuretype = *optarg;
#if DEBUG
	 printf( "Measure type is %c\n", myflags.measuretype );
#endif
	 break;
       case 'v':/* verbose */
	 myflags.verbose = 1;
#if DEBUG
	 printf( "Verbose run\n" );
	 displayFlags( myflags );
#endif
	 break;
       case 'w':/*window length to use for matching*/
	 ret = sscanf( optarg, "%d", &myflags.windowsize );
	 if( ret != 1 )
	   doError( "Invalid argument for max query length\n" );
#if DEBUG
	 printf( "Number of returned files is now %d\n", numReturned );
#endif
	 break;
       case 'W':/* skip weight type */
	 myflags.weighttype = *optarg;
#if DEBUG
	 printf( "Weight type is %c\n", myflags.weighttype );
#endif
	 break;
       case 'M':/* match value */
	 ret = sscanf( optarg, "%d", &myflags.match );
	 matchset = 1;
	 if( ret != 1 )
	   doError( "Invalid argument for quantity returned\n" );
#if DEBUG
	 printf( "Mismatch value is now %d\n", mismatch );
#endif
	 break;
       case 'A':/* small match value */
	 ret = sscanf( optarg, "%d", &myflags.smallmatch );
	 smallmatchset = 1;
	 if( ret != 1 )
	   doError( "Invalid argument for quantity returned\n" );
#if DEBUG
	 printf( "Mismatch value is now %d\n", mismatch );
#endif
	 break;
       case 'D':/* debug level */
	 ret = sscanf( optarg, "%d", &myflags.debuglevel );
	 if( ret != 1 || myflags.debuglevel < 0 )
	   doError( "Invalid argument for debuglevel returned\n" );
	 break;	  
	 /********************** Flags ************************/
       case 'S':/* Skips in queries */
	 myflags.queryskips = 1;
#if DEBUG
	 printf( "Skips in queries\n" );
#endif
	 break;
       case 'I':/* No indels */
	 myflags.noindels = 1;
#if DEBUG
	 printf( "No indels\n" );
#endif
	 break;
       case 'N':/* No indels */
	 myflags.nomismatch = 1;
#if DEBUG
	 printf( "No mismatches\n" );
#endif
	 break;
       case 'Q':/* Query string instead of filename */
	 myflags.querystring = 1;
#if DEBUG
	 printf( "Rests are included\n" );
#endif
	 break;
       default:/* unknown option */
	 printf( "Unknown option %s\n", optarg );
	 showHelp();
	 exit(1);
	 break;
       }
   }

#if DEBUG
 printf( "compresstype has been initialised to %s\n", compresstype );
#endif

 if( myflags.debuglevel > 0 )
   {
     argno = optind;
     while( argno < argc )
       {
	 printf( "And now we are up to argument %d\n", argno );
	 printf( "which is %s\n", argv[ argno++ ] );
       }
   }


 /* And now to business...*/
 /* Check that options are correctly set */
 fixMatchValues( smallmatchset, &myflags ); /* fix incomplete skip options */
 if( myflags.debuglevel > 0 )
   displayFlags( myflags );
 if( myflags.verbose )
   printf( "Melquery version %s\n", version );
 if( myflags.notetype != 'd' && myflags.noindels &&
     myflags.measuretype == 'a' )
   doError( 
	 "noindels (-I) only implemented for local alignment with mod 12\n" );
 /* Check that query file name was actually given first*/
 if( !queryfilename )
   doError( "No query file name given, use option -q queryfilename\n" );

 /* Get the query file */
 if( !myflags.querystring )
   queryPtr = getSongFile( queryfilename, maxQueryLength, myflags );
 if( myflags.debuglevel > 1 && !myflags.querystring )
   displayMelody( queryPtr );
 if( myflags.debuglevel > 0 )
   printf( "maxQueryLength is %d\n", maxQueryLength );


 /* Load the database and process songs */
 if( !database )
   doError( "No database file name given, use option -d databasefilename\n" );
 if( myflags.debuglevel > 0 )
   printf( "about to initialise structure\n" );
 msrsPtr = initResultList();
 if( myflags.debuglevel > 0 )
   printf( "About to get All Measures\n" );
 if( myflags.notetype != 'i' )
      {
       if( !(dbPtr = fopen( database, "r") ) )
	doError( "Error: Can't open database file\n" );
       if( !myflags.querystring )
	querystring = getString( queryPtr, myflags );
       else
	querystring = strdup( queryfilename );
       getCompressedTextMeasures( querystring, dbPtr, msrsPtr, 
				  mismatch, indel, compresstype, myflags   );
      }
 else
  {
   if( myflags.querystring )
    doError( "Error: -Q option is not available for binary songs\n" );
  }
 
 /* Sort and present results */
 sortResults( msrsPtr );
 if( myflags.verbose )
  {
    printf("Using database: %s, measuretype: %c, notetype: %c,\n",
	   database, myflags.measuretype, myflags.notetype );
    printf( "mismatch value: %d, indel value: %d\n", mismatch, indel );
    if( myflags.querystring )
      printf( "query length: %d\n", strlen( querystring ) );
    else
      printf( "query length: %d\n", queryPtr->songLength );
    displayFlags( myflags );
    displayResults( msrsPtr, numReturned, myflags  );
  }
 if( myflags.debuglevel > 0 )
   printf( "Query file name is %s with address %d\n", queryfilename, 
	queryfilename );
 switch( myflags.outputtype)
{
 case 'i':/* Output required for incipit queries */
  displayIncipitFormat( msrsPtr, numReturned, queryfilename, myflags );
  break;
 case 'k':/* Output required for karaoke queries */
    displayKaraokeFormat( msrsPtr, numReturned, queryfilename, myflags );
  break;
 case 'h':/* Output required for humming queries */
   displayHummingFormat( msrsPtr, numReturned, queryfilename, myflags );
  break;
 default:/* unknown option */
  displayRecallFormat( msrsPtr, numReturned, queryfilename, myflags );
	 break;
}
  return 0;
}

void showHelp()
{
  printf( "Command line options\n" );
  printf( "-A arg\t: minor match value - used in special local alignment type A. Default is the same as the normal match value\n" );
  printf( "-a arg\t: absolute or normalisation type\n" );
  printf( "\tl = log songlength normalisation\n" );
  printf( "\t0 = absolute (no normalisation for length)\n" );
  printf( "\t1-9 = the score is divided by the 1-9th root of the songlength\n" );
  printf( "-b\t: use traceback (default is off)\n" );
  printf( "-c arg\t: compress multiple channels/parts into one answer\n" );
  printf( "\tc = compress channels - assumes tncn endings of filenames and sorted\n" );
  printf( "\tp = compress parts - assumes tncnpn endings of filenames and sorted\n" );
  printf( "-d arg\t: database file name containing fully pathed filenames\n" );
  printf( "-e\t: use exhaustive search (default is off) - not implemented\n" );
  printf( "-f\t: use fast preprocessed database format (default is off)\n" );
  printf( "-g arg\t: n-gram size (in range 1-8) or min substring length\n" );
  printf( "-h\t: help - show this information\n" );
  printf( "-I \t: Don't use indels in local alignment\n" );
  printf( "-i arg\t: indel value (negated)- used in local alignment\n" );
  printf( "-k arg\t: maximum number of skipped notes in a match\n" );
  printf( "-l arg\t: maximum query length to use (truncate query)\n" );
  printf( "-M arg\t: match value - used in local alignment\n" );
  printf( "-m arg\t: mismatch value (negated) - used in local alignment\n" );
  printf( "-N \t: Don't use mismatches in local alignment (currently doesn't use indels either)\n" );
  printf( "-n arg\t: note comparison type\n" );
  printf( "\tc = melody contour\n" );
  printf( "\tC = melody contour + rhythm contour\n" );
  printf( "\td = intervals and direction modulo'd to max 1 8ve (default)\n" );
  printf( "\tD = intervals and direction modulo'd to max 1 8ve + rhythm contour\n" );
  printf( "\ti = exact intervals (includes direction) \n" );
  printf( "\tn = all note info (intervals, rhythm and stress) \n" );
  printf( "-p arg\t: number of files to return\n" );
  printf( "-q arg\t: query file name - must be a preprocessed file \n" );
  printf( "-Q \t: query string passed instead of query file name - must be in fast (-f) mode \n" );
  printf( "-r\t: use rests in comparisons (default is off)\n" );
  printf( "-S\t: use skips in queries\n" );
  printf( "-s\t: use stress (default is off) - not implemented\n" );
  printf( "-t arg\t: measure type (default = local alignment)\n" );
  printf( "\ta = local alignment\n" );
  printf( "\tA = Special local alignment with skipped interval calculations\n" );
  printf( "\tb = special longest common subsequence for intervals\n" );
  printf( "\tc = count of distinct common n-grams\n" );
  printf( "\td = sum of frequency of different n-grams\n" );
  printf( "\tf = end-space free alignment\n" );
  printf( "\tg = global alignment\n" );
  printf( "\tG = substring alignment with gaps\n" );
  printf( "\th = hybrid alignment (global but initialised as local)\n" );
  printf( "\tI = local alignment without indels\n" );
  printf( "\ti = tf * idf\n" );
  printf( "\tL = tf * log idf\n" );
  printf( "\tl = longest common subsequence\n" );
  printf( "\tM = local alignment without mismatches or indels\n" );
  printf( "\tm = mixed alignment\n" );
  printf( "\tn = sum of frequency of common n-grams\n" );
  printf( "\tq = full query alignment\n" );
  printf( "\tr = log( 1 + tf ) * idf (diminishing returns) \n" );
  printf( "\ts = longest substring, with minimum g substring length or 0\n" );
  printf( "\tt = weighted count of distinct common n-grams\n" );
  printf( "\tu = difference in frequency of all n-grams (Ukkonen)\n" );
  printf( "\tw = local alignment with increasing contiguous match weights\n" );
  printf( "\t2 = combination of sums of weighted sums and differences of n-grams\n" );
  printf( "-v\t: verbose - show extra information\n" );
  printf( "-W arg\t: weight type used for skip matching\n" );
  printf( "\ta = arithmetic weights for all skips\n" );
  printf( "\te = equal weights for all skips\n" );
  printf( "\tg = geometric progression weights for all skips\n" );
  printf( "-w arg\t: window size used for matching, 0 for full string\n" );
}


