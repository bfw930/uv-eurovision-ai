/* File: meldb.c
   Author: Sandra Uitdenbogerd
   Date: 19 July 99
   This file contains the functions that used to exist in
   the file melquery.c.
   They have been put into this file to allow the development of
   a group query program that permits users to submit a collection
   of queries to be processed in the same manner.
   Updates in 2005 include calls to mixedalignment function
*/
#define debug 0
#define DEBUG 0
#define READLOGS 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/limits.h>

/*#include "top.h" */
#include "melodies.h" 
#include "melsim.h"
#include "meldb.h"
#include "prefile.h"
#include "melstrings.h"

#include "general.h"



RESULTLIST *initResultList()
{
RESULTLIST *resultlist;

/* Create the result list structure */
if( ! (resultlist = (RESULTLIST *) malloc( sizeof( RESULTLIST ) ) ) )
   doError( "Failed to allocate space for RESULTLIST structure\n" );
resultlist->numSongs = 0;
if( ! (resultlist->result = (RESULT **) malloc( sizeof( RESULT *) * SIZE ) ) )
   doError( "Failed to allocate space for RESULTLIST structure's notes\n" );
resultlist->storeLength = SIZE;

return resultlist;

}

void *initMeasureStructure( char *queryArray, 
			    int queryLength, int *ngramarrSize,
			    FLAGS myflags )
{
  /*
    This initialises the appropriate structure for the type of measure and
    note being used.
  */
void *structurePtr;

 if( myflags.debuglevel > 0 )
  {
    printf( "initMeasureStructure called\n" );
  }
 switch( myflags.measuretype )
   {
    case 'u':
    case 'n':
    case 'c':
    case 't':
    case 'i':
    case 'L':
    case 'r':
      if( useNGramArr( myflags ) )
	structurePtr = (void *) setupNgramCount( queryArray, queryLength,
			 ngramarrSize, myflags );
      else
	structurePtr = (void *) setupNgramTree( queryArray, ngramarrSize, 
			  queryLength, myflags );
      break;
    case 'b':
      structurePtr = (void *) setupIntervalGrid( queryArray, queryLength, 
						 myflags );

      if( myflags.debuglevel > 0 )
	{
	  printf( "query UChar array is equal to ...\n" );
	  showUChars( queryArray, queryLength );
	  printf( "query interval grid is set to...\n" );
	  showIntervalGrid( (short **)structurePtr, queryLength );
	}
      break;
   }
 return structurePtr;
}

double getnormFactor( int strlength, FLAGS myflags )
{
if( myflags.debuglevel > 0 )
  {
    printf( "getnormFactor called. \n" );
    printf( "strlen = %d, normlog = %d and normroot = %d\n",
	strlength, myflags.normlog, myflags.normroot );
  }
  if( myflags.normlog ) /* use log normalisation */
    return ( log( strlength ) + 1.0 ) / 20.0 ;
  else if( !myflags.normroot )
    return 1.0;
  else if( myflags.normroot == 1 )
    return strlength/20.0;
  else
    return powf( strlength, 1.0/myflags.normroot )/ 20.0;
}

char *createIDFFileName( FLAGS myflags, char filetype ) 
{
  /* create the filename to be used for loading idf values
     In this the new version, the file name is:
     path, "idf", n-gramsize, value type, database root, ".", "txt" or "bin"
     eg. idf5lamdbd3.bin
   */
 int spot;
  char *idffilename, *charptr, *cptr, *dbname;

  dbname = strdup( getRoot( myflags.database, "n" ) );
  if( !( idffilename = 
	 (char *) malloc( (strlen( IDFFILE ) 
			   + strlen( dbname ) + 7) * sizeof( char )  ) ) )
    doError( "Failed to allocate string for IDF filename\n");
if( myflags.debuglevel > 0 )
  {
  printf( "malloc'd filename space\n" );
  }
  if( !( idffilename = strcpy( idffilename, IDFFILE ) ) )
    doError( "Failed to concatenate string for IDF filename\n" );
  spot = strlen(idffilename);
  if( sprintf( idffilename + spot++, "%d", myflags.ngramsize ) != 1 )
    doError( "Didn't write exactly one char for ngramsize in idf filename\n" );
  idffilename[spot++] = myflags.measuretype;
  if( !( strcpy( idffilename + spot, dbname ) ) )
    doError( "Failed to copy dbname to the idf filename \n" );
  spot += strlen( dbname );
  switch( filetype )
    {
    case 'a':
    case 't':
      cptr = strcpy(idffilename + spot, ".txt" ); 
      break;
    case 'b':
      cptr = strcpy(idffilename + spot, ".bin" ); 
      break;
    default:
      doError( "Error: I don't know about this file-type\n" );
    }
if( myflags.debuglevel > 0 )
  printf( "idffilename is %s\n", idffilename );
return idffilename;
}

double *initngramfreqs( int *numElts, FLAGS myflags )
{
  /* This allocates the memory and initialises elements to 0 */
  int i;
  double *ngramfreqs;

  if( myflags.debuglevel >= 1 )
    printf( "initngramfreqs called\n" );
  *numElts = pow( alphasize( myflags.notetype ), myflags.ngramsize );
  if( myflags.debuglevel >= 1 )
    printf( "About to malloc numElts = %d\n", *numElts );
  if( !(ngramfreqs = (double *) malloc( *numElts * sizeof( double ) ) ) )
    doError( "Error: Failed to allocate space for IDF frequencies\n" );
  if( myflags.debuglevel >= 1 )
    printf( "Successfully malloc'ed numElts = %d\n", *numElts );
  for( i=0; i < *numElts; i++ )
    {
        if( myflags.debuglevel >= 2 )
	  printf( "currently setting element %d\n", i );
      ngramfreqs[i] = 0.0;
    }
  if( myflags.debuglevel >= 1 )
    printf( "Successfully initialised all elements to 0\n" );
  return ngramfreqs;
}

void showDocFreqs( double *docfreqs, int numelts )
{
int i;

printf( "Printing inverse document frequencies\n" );
for( i=0; i < numelts; i++ )
  if( docfreqs[i] != 0 )
    printf( "%d\t%f\n", i, docfreqs[i] );
}

double *loadDocFreqs( char *filename, int *size, FLAGS myflags )
{
  /* Unlike initDocFreqs, this function reads a binary file into memory
     which maps straight to an array of doubles 
  */

  double *docFreqs;
  int filesize;

  if( myflags.debuglevel )
    printf( "loadDocFreqs called\n" );
  docFreqs = (double *) putFileInMemory( filename, &filesize, myflags );
  *size = filesize;
  if( myflags.debuglevel >= 1 )
    printf( "docFreqs loaded with filesize = %d\n", filesize );
  return docFreqs;
}


double *initDocFreqs( FLAGS myflags, int *elts )
{
  /*
    In this function we initialise a structure that will contain the
    inverse document frequency for each term.
    This function reads a text file containing the doc freq data
  */
  char *startpt, ngram[5], *endpt;
  int filesize, spot, ret, dbsize, remainder, numElts;
  char *idffilename, *charptr;
  double *ngramfreqs;
  char *ptrtemp;
#if READLOGS
  double ngramfreq;
#else
  int ngramfreq;
#endif
  int i = 0;

#define SKIPMAX 50
if( myflags.debuglevel > 0 )
  printf( "Starting initDocFreqs.  This only works for modulo int and small n-grams\n" );
  if( myflags.notetype != 'd' || myflags.ngramsize >= 5 )
    doError( "Error: TF*IDF not yet implemented for this option\n" );
  idffilename = createIDFFileName( myflags, 'a' );
  /* Open the file of terms and frequencies */
  startpt = putFileInMemory( idffilename, &filesize, myflags );
  /* Read the values into the structure */
if( myflags.debuglevel > 0 )
  printf( "putFileInMemory completed\n" );
 charptr = startpt;
 ret = sscanf( charptr, "%d", &dbsize );
if( myflags.debuglevel > 0 )
  printf( "sscanf of dbsize completed, dbsize = %d\n", dbsize );
 charptr = memchr( charptr, '\n', filesize );
if( myflags.debuglevel > 0 )
  printf( "memchr completed\n");
 charptr++;
 endpt = startpt +  filesize;
  /*  remainder = filesize -  (int) charptr +  (int) startpt;*/
  ngramfreqs = initngramfreqs( &numElts, myflags );
if( myflags.debuglevel > 0 )
  {
    printf( "startpt is %d, endpt is %d, filesize is %d, charptr is %d\n", 
	startpt, endpt, filesize, charptr );
    printf("ret is %d\n", ret );
  }
 while( charptr && ret > 0 && charptr <  endpt-4  )
   {
     if( myflags.debuglevel > 2 )
       printf( "about to sscanf\n" );
     /*            ret = sscanf( charptr, "%4s", ngram ); */
     ptrtemp = memcpy( ngram, charptr, 4 );
     ngram[4] = '\0';
     charptr += 4;
     if( myflags.debuglevel > 2 )
       printf( "charptr is now %d\n", charptr );
     if( charptr < endpt - 2 )
       {
	 charptr = memchr( charptr, ' ', SKIPMAX );
     if( myflags.debuglevel > 2 )
	  printf( "ngram %s read, ret is %d, about to sscanf the frequency\n", 
		  ngram, ret );
      /*      printf("next 20 chars are: %20s\n", charptr );*/
	  ret = sscanf( charptr, "%lf", &ngramfreq );
	  if( ret > 0 )
	    {
     if( myflags.debuglevel > 1 )
	      printf( "%d ngram is %s ngramfreq is %f\n", i, ngram, ngramfreq );
	      ngramfreqs[ nghashMod( ngram, myflags.ngramsize )] = ngramfreq;
	      charptr = memchr( charptr, '\n', filesize );
	      charptr++;
     if( myflags.debuglevel > 1 )
       {
	      printf( "log value is %f\n",  ngramfreq  );
	      printf( "charptr is %d and endpt is %d\n", charptr, endpt );
       }
      /*      remainder = filesize - (int) charptr + (int) startpt;    */
	      i++;
	    }
	}
    } /* end while */
  if( munmap( startpt, filesize ) )
    fprintf( stderr, "Problem calling munmap\n" );
  if( myflags.debuglevel > 0 )
    {
  printf( "dbsize read as %d, numElts is %d\n", dbsize, numElts );
  showDocFreqs( ngramfreqs, numElts );
    }
  *elts = numElts;
  return ngramfreqs;
}

void addToTree( char *ngram, double idfvalue, NGRAMTREE *ngramtree,
		int arrsize, FLAGS myflags )
{
  NGRAMTREE *ptr, *newEntry, *previous;
  int left;

  if( myflags.debuglevel > 1 )
    printf( "addToTree called\n" );
  ptr = ngHashUChar( ngram, ngramtree, arrsize, &previous, &left, 
		       myflags.ngramsize );
  if( !ptr )
    {
      if( myflags.debuglevel > 1 )
	printf( "No existing node, so create one\n" );
      /* null pointer, so add an entry */
      if( !(newEntry = (NGRAMTREE *) malloc( sizeof( NGRAMTREE ) ) ))
	doError( "Failed to create new hash overflow entry" );
      newEntry->exists = 0;
      newEntry->left = NULL;
      newEntry->right = NULL;
      newEntry->part2value = strdup(ngram + 2);
      newEntry->idf = idfvalue;
      if( myflags.debuglevel > 1 )
	{
	  printf( " part2value is" );
	  showUChars( newEntry->part2value, myflags.ngramsize - 2 );
	}
      if( previous )
	if( left )
	  previous->left = newEntry;
	else
	  previous->right = newEntry;
    }
  else /* adding to existing node */
    {
      if( myflags.debuglevel > 2 )
	printf( "ptr is %li\n", ptr );
      if( !ptr->part2value )
	  ptr->part2value = strdup( ngram + 2 );
      ptr->idf = idfvalue;
      if( myflags.debuglevel > 2 )
	printf( "existing array element part2value is %d\n", 
		newEntry->part2value );
    }
}

void loadTreeDocFreqs( char *filename, NGRAMTREE *ngramtree, int arrsize, 
		       FLAGS myflags )
{
  /* load the n-gram tree with each ngram and idf value */
  FILE *binfile;
  int numsongs;
  char *ngrambuffer;
  double idfvalue;
  int buffersize;

  if( myflags.debuglevel )
    printf( "loadTreeDocFreqs called with filename %s\n", filename );
  buffersize = myflags.ngramsize + 1;
  if( ! ( ngrambuffer = (char *) malloc( sizeof( char ) 
                                         * ( buffersize ) ) ) )
     doError( "Failed to allocate space for ngram buffer\n" );
  if( !( binfile = fopen( filename, "r" ) ) )
    doError( "Failed to open binary file\n" );
  if( !(fread( &numsongs, sizeof(int), 1, binfile ) == 1 ) )
    doError( "Failed to read number of songs\n" );
  if( myflags.debuglevel )
    printf( "Number of songs is %d\n", numsongs );
  while( ( fread( ngrambuffer, buffersize * sizeof( char ), 1, 
		binfile )  == 1 ) )
    {
      if( !( fread( &idfvalue, sizeof( double ), 1, binfile ) ) == 1 )
        doError( "Failed to read idf value\n" );
      addToTree( ngrambuffer, idfvalue, ngramtree, arrsize, myflags );
      if( myflags.debuglevel > 1 )
	printf( "%s\t%lf\n", ngrambuffer, idfvalue );
    }
}

void getCompressedTextMeasures( char *querystring, FILE *dbPtr, 
				RESULTLIST *resultlist,
		     int mismatch, int indel, char *compresstype,
		     FLAGS myflags   )
{
char *melfilename, *retval, *songstring, *buffer;
char *lastSongRoot = NULL, *thisSongRoot = NULL;
FILE *melfilePtr;
int i, allocCount = 1, tempscore, queryLength, songLength;
int arrsize, numElts, filesize;
double normaliser;
void *measureStructurePtr;
double *docfreqs;
 MATCHINFO tempmatch;
/* 
   This function is only called when the file being processed consists
   of readable strings (fast mode) and using contour or modulo intervals.
*/
 if( myflags.debuglevel > 0 )
   printf( "getCompressedTextMeasures called\n" );

queryLength = strlen( querystring );
measureStructurePtr =  initMeasureStructure( querystring, queryLength,
			  &arrsize, myflags );
 if( myflags.debuglevel )
   printf( "back from initmeasurestructure\n" );
/* if this is a tf*idf kind of measure */
if( myflags.measuretype == 'i' ||myflags.measuretype == 'r' 
    || myflags.measuretype == 'L' )
  {
    if( myflags.ngramsize <= 4 )
      {
	numElts = (int) pow( (double) alphasize( myflags.notetype ), 
			 (double) myflags.ngramsize );
	if( myflags.debuglevel )
	  printf( "numelts calculated as %d\n", numElts );
	docfreqs = loadDocFreqs( createIDFFileName( myflags, 'b' ), &filesize, 
			     myflags );
	if( myflags.debuglevel > 2 )
	  showDocFreqs( docfreqs, numElts );
      }
    else
      {
	loadTreeDocFreqs( createIDFFileName( myflags, 'b' ), 
			  measureStructurePtr, arrsize, myflags );
	if( myflags.debuglevel > 1 )
	  {
	    if( myflags.notetype == 'd' || myflags.notetype == 'c' )
	      showFormattedNgramTree( measureStructurePtr, arrsize, 
				      myflags.ngramsize, myflags );
	    else
	      showNgramTree( measureStructurePtr, arrsize, 
			     myflags.ngramsize );
	    printf( "longest tree branch is %d\n", 
		    longestNgramTreeBranch( measureStructurePtr, arrsize ) );
	  }
      }
  }	
 if( myflags.debuglevel > 0 )
   printf( "Query string is: %s at %d with %d length\n", 
	   querystring, querystring, queryLength );
 
/* for each filename in the database file
 call the compare function to compare the query string with this one.
Store the melody file name and the comparison score in the resultlist
structure */
i = 0;
buffer = fgetLargeString( dbPtr );
while( buffer )
  {
    melfilename = buffer;
    songstring = strchr( buffer, ' ' );
    if( !songstring )
      doError( 
	"Error: no space in string database line between name and melody\n" );
    *songstring = '\0';
    songstring++;
 if( myflags.debuglevel > 0 )
    printf( "This song is %s, compresstype is %s\n", melfilename, 
	    compresstype );
    resultlist->result = (RESULT **) allocArrayElement( 
	   (int *) &(resultlist->storeLength ), 
	   (void *) resultlist->result, SIZE, 
	   sizeof( RESULT **), sizeof( RESULT ), i );
    if( ! (resultlist->result[i]->matchinfo = (MATCHINFO *) 
	   malloc( sizeof( MATCHINFO ) ) ) )
     doError( "Failed to allocate space for RESULTLIST structure\n" );
    thisSongRoot = getRoot( melfilename, compresstype );
    resultlist->result[i]->songName = thisSongRoot;
    normaliser = getnormFactor( strlen( songstring ), myflags );
    if( myflags.debuglevel > 0 )
      printf( "normaliser for song %s with length %d is %f", 
	      thisSongRoot, strlen( songstring ), normaliser );
    resultlist->result[i]->score =   getMeasure( querystring, 
	   queryLength,   songstring, strlen( songstring ), 
	   mismatch, indel, measureStructurePtr, arrsize, docfreqs, myflags, 
	  resultlist->result[i]->matchinfo ) / normaliser ;
    if( myflags.debuglevel > 1 )
     {
      printf( "%s %s\n", melfilename, thisSongRoot );
     }
    if( myflags.debuglevel > 0 )
     {
      printf( "i=%d thisSongRoot = %s and lastSongRoot = %s\n", i, 
	      thisSongRoot, lastSongRoot );
      printf( "Address of thisSongRoot = %d, and lastSongRoot = %d\n",
	    thisSongRoot, lastSongRoot );
      printf( "Address of current song name is %d\n",
	    resultlist->result[i]->songName );
     }
 if( lastSongRoot && !strcmp(thisSongRoot, lastSongRoot) )
      {
	if( resultlist->result[i]->score > resultlist->result[i-1]->score)
	  {
	    resultlist->result[i-1]->score = resultlist->result[i]->score;
	  }
      }
    else
      {
	i++;
      }
    /* There is a memory leakage here that I haven't fixed - to do with
       song root names */
    lastSongRoot = thisSongRoot;
    free( buffer );
    buffer = fgetLargeString( dbPtr );
  }
free( querystring );
fclose( dbPtr );
resultlist->numSongs = i;
}

NGRAMS *setupNgramCount( char *querystring, int qlen, int *asize,
			 FLAGS myflags )
{
  /* 
     This sets up an array structure for a unique hash table for n-grams.
     As such, it can only be used for short ngrams or small alphabetsize.
  */
int arrsize = pow( (alphasize( myflags.notetype)) ,  myflags.ngramsize);
NGRAMS *arr = malloc( sizeof( NGRAMS) * arrsize );

 if( myflags.debuglevel )
   printf( "setupNgramCount called\n" );
 if (!arr )
  doError( "Failed to allocate space for hash table\n" );
 initngramarr( arr, arrsize, myflags );
#if DEBUG
printf( "counting for query \n" );
if( myflags.notetype == 'i' )
  showUChars( querystring, qlen );
else
  printf( "%s\n", querystring );
printf( "Array size is %d\n", arrsize );
showNgramArray( arr, arrsize );
#endif
switch( myflags.notetype )
  {
  case 'd':
    scanquery( arr, arrsize, querystring, qlen, nghashMod, myflags );
    break;
  case 'c':
    scanquery( arr, arrsize, querystring, qlen, nghashContour, myflags );
    break;
  case 'i':
    scanquery( arr, arrsize, querystring, qlen, nghashUCharArr, myflags );
    break;
  default:
    doError( "Only count contour and modulo int ngrams\n" );
  }
#if DEBUG
printf( "After scanquery...\n" );
showNgramArray( arr, arrsize );
#endif
*asize = arrsize;
 return arr;
}

short **setupIntervalGrid( unsigned char *querystring,
			   int querylength, FLAGS myflags )
{
  /* This function allocates the memory for a 2-D array that is used
     for storing interval sums of a query or song.
     It then fills the array with interval sums based on the char array 
     passed as the first argument.
  */
short **ptr, **grid;
register int i,j;


if( !(grid = malloc( sizeof( short * ) * querylength ) ) )
  doError( "Failed to allocate pointers for interval grid\n" );
ptr = grid;
for( i=0; i < querylength; i++ )
  {
    if( !(*ptr = malloc( sizeof( short ) * querylength ) ) )
      doError( "Failed to allocate space for interval grid\n" );
    ptr++;
  }
for( i = 0; i < querylength; i++ )
  for( j = i; j < querylength ; j++ )
    {
      if( i == j )
	grid[i][j] = querystring[i] - 127;
      else
	grid[i][j] = querystring[j] - 127 + grid[i][j-1];
    }
return grid;
}


void freeIntervalGrid( short **grid, int querylength )
{
  /* This function frees the memory of a 2-D array that is used
     for storing interval sums of a query or song
  */
register int i;
short **ptr = grid;

for( i=0; i <= querylength; i++ )
  free( *ptr++ );
free( grid ); 
}

NGRAMTREE *setupNgramTree( unsigned char *querystring, int *asize, 
			   int querylength, FLAGS myflags )
{
int arrsize = pow( (alphasize( 'i')) , 2);
NGRAMTREE *arr = malloc( sizeof( NGRAMTREE) * arrsize );

 if( myflags.debuglevel )
   printf( "setupNgramTree called\n" );

 if (!arr )
  doError( "Failed to allocate space for hash table\n" );
 initUCharngramarr(  arr,  arrsize );
 if( myflags.debuglevel > 1 )
   {
     printf( "counting for query\n" );
     showUChars( querystring, querylength );
     printf( "Array size is %d\n", arrsize );
     showNgramTree( arr, arrsize, myflags.ngramsize );
   }
 scanUCharquery( arr, arrsize, querystring , querylength, myflags );
 if( myflags.debuglevel > 1 )
   {
     printf( "After scanquery...\n" );
     showNgramTree( arr, arrsize, myflags.ngramsize );
   }
if( myflags.verbose || myflags.debuglevel )
  printf( "longest tree branch is %d\n", 
	  longestNgramTreeBranch( arr, arrsize ) );
*asize = arrsize;
 return arr;
}

int useNGramArr( FLAGS myflags )
{
switch( myflags.notetype )
  {
  case 'c':
    return 1;
    break;
  case 'C':
  case 'd':
    return (myflags.ngramsize <= 4 );
    break;
  case 'D': 
    return (myflags.ngramsize <= 3 );
    break;
  case 'i':
  case 'I':
    return (myflags.ngramsize <= 2 );
    break;
  default:
    break;
  }
}

double getMeasure(char *querystring, int queryLength, char *songstring, 
		int songLength,
	       int mismatch, int indel, void *measureStructurePtr, int arrsize,
		double *docfreqs, FLAGS myflags, MATCHINFO *tempmatch )
{
  /*
    This function is called for contour and modulo note-types to calculate
    the score.
    If other note-types are used, then the useArr flag may need to be changed.
  */
double score;
int qpos, spos, diff;
char *querykeystring, *songkeystring, *transquery;
int (*fptr)( char *, int );
int useArr;

 if( myflags.debuglevel > 0 )
   printf( "getMeasure called \n" );
useArr = useNGramArr( myflags );
switch( myflags.measuretype )
  {
  case 'l':
    score = smallLCS( querystring, 
		       songstring, myflags );
    break;
  case 'u':
  case 'c':
    if( useArr )
      {    
	fptr = gethashfn( myflags.notetype );
	resetngramarr( measureStructurePtr, arrsize, querystring, 
		       queryLength, fptr,
		       myflags );
      }
    else
      {
	resetUCharngramarr( measureStructurePtr, arrsize, querystring, 
			    queryLength, myflags );
      }
#if DEBUG
    printf( "After resetting the array:\n" );
    showNgramArray( measureStructurePtr, arrsize );
#endif
  case 'n':
  case 't':
  case 'i':
  case 'L':
  case 'r':
    if( useArr )
      score = countngrams( measureStructurePtr, arrsize, 
		     songstring, docfreqs, 
		     querystring, queryLength, songLength,
		     docfreqs, myflags );
    else
      score = scanUChartext( measureStructurePtr, arrsize, songstring, 
			     songLength, myflags, queryLength );
    break;
  case 'b':
#if POS
    score = poslocalalignment( querystring, 
		       songstring, mismatch, indel, &qpos, &spos, myflags );
#if DEBUG
    printf( "qpos = %d, spos = %d, score = %d\n", qpos, spos, score );
#endif
    querykeystring = keystring( querystring );
    songkeystring = keystring( songstring );
    diff = querykeystring[qpos] - songkeystring[spos];
    transquery = transpose( querykeystring, diff );
    score = smallLCS( transquery, songkeystring, myflags );
#endif
    break;
  case 'g': 
    score = globalalignment( querystring, songstring, mismatch, indel, myflags );
    break;
  case 'h': 
    score = hybridalignment( querystring, songstring, mismatch, indel, myflags );
    break;
  case 'm': 
   if( myflags.traceback )
    {
     score = mixedtbalignment( querystring, songstring, mismatch, indel, 
			       myflags, tempmatch );
    }
   else
    {
     score = mixedalignment( querystring, songstring, mismatch, indel, myflags );
    }
    break;
  case 'f': 
    score = endspacefreealignment( querystring, songstring, mismatch, indel, myflags );
    break;
  case 'q': 
    score = fullqueryalignment( querystring, songstring, mismatch, indel, myflags );
    break;
  case 'Q': 
    score = globalqueryalignment( querystring, songstring, mismatch, indel, myflags );
    break;
  case 'A': /* local alignment with 1-2 skips */
    score = specLocalAlignment( querystring, songstring, mismatch, indel, 
				myflags );
    break;
  case 'I':
  case 'M': /* matches/mismatches only */
      score = noIndelLocalalignment( querystring, 
		       songstring, mismatch, indel, myflags );
    break;
  case 's': /* longest substring with minimum length run for scores */
    score = longestSubstring( querystring, 
		       songstring, mismatch, indel, myflags );
    break;
  case 'G':
    score = longestSubstringWithGaps( querystring, 
		       songstring, mismatch, indel, myflags );
    break;
  case 'w': /* weight to add increases as length of match increases */
    score = bizarroLocalAlignment( querystring, 
		       songstring, mismatch, indel, myflags );
    break;
  case 'a': /* normal local alignment */
  default:
      score = smalllocalalignment( querystring, 
		       songstring, mismatch, indel, myflags );
    break;
  }
if( myflags.debuglevel > 2 ) 
 printf( "getMeasure returning a score of %.2f\n", score );
return score;
}

double getUCharMeasure(unsigned char *querystring, unsigned char *songstring, 
		     int qlen, int slen, 
		     int mismatch, int indel, void *measureStructurePtr, 
		     int arrsize, double *docfreqs,
		     FLAGS myflags )
{
  /* 
     This gets the similarity score for a pair of uchar arrays.
     If n-grams are used, then the n-gram length determines whether
     an ngram array or tree is used.  Lengths of 2 or less use an array.
  */
double score;
int qpos, spos, diff;
char *querykeystring, *songkeystring, *transquery;
short **songgrid;
int (*fptr)( char *, int );
int useArr = myflags.ngramsize <= 2;

#if DEBUG
printf( "getUCharMeasure called with querylength = %d and songlength = %d\n", 
	qlen, slen );
printf( "querystring is\n" );
showUChars( querystring, qlen );
printf( "songstring is\n" );
showUChars( songstring, slen );
#endif
switch( myflags.measuretype )
  {
  case 'l':
      score = uCharLCS( querystring, 
		       songstring, qlen, slen );
      break;
  case 'u':
  case 'c':
    if( useArr )
      {    
	fptr = gethashfn( myflags.notetype );
	resetngramarr( measureStructurePtr, arrsize, querystring, qlen, fptr,
		       myflags );
      }
    else
      {
	resetUCharngramarr( measureStructurePtr, arrsize, querystring, 
			    qlen, myflags );
      }
  case 'n':
  case 't':
  case 'i':
  case 'L':
  case 'r':
    if( useArr )
      score = countngrams( measureStructurePtr, arrsize, 
		     songstring, docfreqs,
			   querystring, qlen, slen, docfreqs, myflags );
    else
      score = scanUChartext( measureStructurePtr, arrsize, songstring, 
			     slen, myflags, qlen );
    break;
    case 'b':
#if DEBUG
printf( "Calling setupIntervalGrid for songstring" );
#endif
      songgrid = setupIntervalGrid( songstring, slen, myflags );
#if DEBUG
      printf( "song interval grid is set to...\n" );
      showIntervalGrid( songgrid, slen );
#endif
      score = specLCS( querystring, songstring, qlen, slen, 
		       measureStructurePtr, songgrid );
      freeIntervalGrid( songgrid, slen );
      break;
  case 'A':
    score = specUCharLocalAlignment( querystring, songstring, qlen, slen,
				     mismatch, indel, myflags );
    break;
  case 'a':
  default:
      score = uCharlocalalignment( querystring, 
		       songstring, qlen, slen, mismatch, indel, myflags );
      break;
  }
return score;
}

void showIntervalGrid( short **arr, int length )
{
int i, j;

for( i=0; i < length; i++ )
  {
    for( j=0; j < i; j++ )
      printf( "    " );
    for(; j < length; j++ )
      printf( "%4ld", arr[i][j] );
    printf( "\n" );
  }

}

char *keystring( char *str )
{
int newvalue;
char prev = '\0';
char *newstr;

if( ! (newstr = (char *) malloc( (sizeof( char ) * strlen( str ) + 2) ) ) )
  doError( "Failed to allocate space for keystring\n" );

while( *str )
  {
    newvalue = *str; 
    if( newvalue > 255 || newvalue <= 0 )
      {
	printf( "newvalue = %d,  str char = %d\n", 
		newvalue, *str );
	doError( "transposed string out of bounds\n" );
      }
    *newstr = newvalue;
    str++;
  }
return newstr;
}

char *transpose( char *querystring, int diff )
{
int i, newvalue;
char *newstr = strdup( querystring );

while( *querystring )
  {
    newvalue = *querystring - diff; 
    if( newvalue > 255 || newvalue <= 0 )
      {
	printf( "newvalue = %d, diff = %d, querystring char = %d\n", 
		newvalue, diff, *querystring );
	doError( "transposed string out of bounds\n" );
      }
    *newstr = newvalue;
    querystring++;
  }
return newstr;
}

int scoreCmp( RESULT **res1, RESULT **res2 )
{
  double temp = ((*res2)->score - (*res1)->score);
  if( temp > 0 )
    return 1;
  else if( temp == 0 )
    return 0;
  else 
    return -1;
}

void sortResults( RESULTLIST *results )
{
qsort( results->result, results->numSongs, sizeof( RESULT *),
       (int (*)(const void *, const void *))scoreCmp );
}

void displayFlags( FLAGS myflags )
{
printf( "Flags used: " );
if( myflags.rests )
  printf( "rests ");
if( myflags.rhythm )
  printf( "rhythm ");
if( myflags.stress )
  printf( "stress ");
if( myflags.verbose )
  printf( "verbose ");
if( myflags.fast )
  printf( "fast ");
if( myflags.queryskips )
  printf( "query skips ");
if( myflags.querystring )
  printf( "query string ");
if( myflags.noindels )
  printf( "no indels ");
if( myflags.traceback )
  printf( "traceback ");
printf( "\n" );
if( myflags.normlog )
  printf( "log normalisation\n");
else
  printf( "normalisation factor: %d\n", myflags.normroot );
printf( "n-gram size: %d, matching window size: %d, max skips %d\n", 
  myflags.ngramsize, myflags.windowsize, myflags.maxskips );
printf( "note type: %c, skip weight type: %c\n", myflags.notetype, myflags.weighttype );
printf( "match: %d, smallmatch: %d, mismatch: %d, indel: %d\n",
	 myflags.match, myflags.smallmatch, myflags.mismatch, myflags.indel);
printf( "debug output level: %d, output type: %c\n", myflags.debuglevel,
	myflags.outputtype );
}

void displayResults( RESULTLIST *results, int numToDisplay, FLAGS myflags )
{
  int i;
  int limit = min( numToDisplay, results->numSongs );

  printf( "Database has %d songs\n", results->numSongs );
  for( i=0;i < limit; i++ )
   {    
    printf( "%d\t%8.2f\t%s", i, results->result[i]->score, 
	    results->result[i]->songName );
    if( myflags.traceback )
     printf( "\t%d\t%d\n", results->result[i]->matchinfo->qbestpos,
	     results->result[i]->matchinfo->sbestpos );
    else
     printf( "\n" );
   }
}



void displayRecallFormat( RESULTLIST *results, int numReturned, 
			  char *queryname, FLAGS myflags )
{
  int i, max;
  char *songroot;

  max = ((numReturned < results->numSongs) ? numReturned: results->numSongs ); 
  if( myflags.verbose )
    printf( "Database has %d songs.  We will print %d of them\n", 
	    results->numSongs, max );
  songroot = strrchr( queryname, '/' );
  if( songroot ) /* just use the root of the name - leave off directory info */
    printf( "%s", ++songroot );
  else
    printf( "%s ", queryname );
  for( i=0;i < max; i++ )
    {
      songroot = strrchr( results->result[i]->songName, '/' );
      if( songroot) 
	printf( " %s", ++songroot );
      else
	printf( " %s", results->result[i]->songName );
    }
  printf( "\n" );
}

void displayIncipitFormat( RESULTLIST *results, int numReturned,
			 char *queryname, FLAGS myflags )
{
  int i, max;
  char *songroot;

  max = ((numReturned < results->numSongs) ? numReturned: results->numSongs ); 
  if( myflags.verbose )
   {
    printf( "displayIncipitFormat called, Database has %d songs.  We will print %d of them\n", 
	    results->numSongs, max );
    /* print query name */
    songroot = strrchr( queryname, '/' );
    if( songroot ) /* just use the root of the name - leave off directory info */
    printf( "%s\n", ++songroot );
  else
    printf( "%s\n", queryname );
     }
  /* print answer names */
  for( i=0;i < max; i++ )
    {
      songroot = strrchr( results->result[i]->songName, '/' );
      if( songroot) 
	printf( "%s\n", ++songroot );
      else
	printf( "%s\n", results->result[i]->songName );
    }
}

void displayKaraokeFormat( RESULTLIST *results, int numReturned,
			 char *queryname, FLAGS myflags )
{
  int i, max;
  char *songroot;

  max = ((numReturned < results->numSongs) ? numReturned: results->numSongs ); 
  if( myflags.verbose )
    printf( "displayKaraokeFormat called, Database has %d songs.  We will print %d of them\n", 
	    results->numSongs, max );
  songroot = strrchr( queryname, '/' );
  if( songroot ) /* just use the root of the name - leave off directory info */
    printf( "%s", ++songroot );
  else
    printf( "%s ", queryname );
  for( i=0;i < max; i++ )
    {
      songroot = strrchr( results->result[i]->songName, '/' );
      if( songroot) 
	printf( " %s", ++songroot );
      else
	printf( " %s", results->result[i]->songName );
    }
  printf( "\n" );
}

void displayHummingFormat( RESULTLIST *results, int numReturned,
			 char *queryname, FLAGS myflags )
{
  int i, max;
  char *songroot;

  max = ((numReturned < results->numSongs) ? numReturned: results->numSongs ); 
  if( myflags.verbose )
    printf( "displayHummingFormat called, Database has %d songs.  We will print %d of them\n", 
	    results->numSongs, max );
  songroot = strrchr( queryname, '/' );
  if( songroot ) /* just use the root of the name - leave off directory info */
    printf( "%s: ", ++songroot );
  else
    printf( "%s: ", queryname );
  for( i=0;i < max; i++ )
    {
      songroot = strrchr( results->result[i]->songName, '/' );
      if( songroot) 
	printf( " %s", ++songroot );
      else
	printf( " %s", results->result[i]->songName );
      if( i < max - 1 )
       printf( "," );
    }
  printf( "\n" );
}


void initFlags( FLAGS *thestruct )
{
  thestruct->rests = 0;
  thestruct->rhythm = 0;
  thestruct->stress = 0;
  thestruct->normlog = 1;
  thestruct->normroot = 0;
  thestruct->verbose = 0;
  thestruct->fast = 0;
  thestruct->traceback = 0;
  thestruct->ngramsize = 4;
  thestruct->notetype = 'd';
  thestruct->outputtype = 'r';
  thestruct->measuretype = 'a';
  thestruct->windowsize = 0;
  thestruct->debuglevel = 0;
  thestruct->maxskips = 0;
  thestruct->match = 1;
  thestruct->smallmatch = 1;
  thestruct->mismatch = -1;
  thestruct->indel = -2;
  thestruct->noindels = 0;
  thestruct->queryskips = 0;
  thestruct->querystring = 0;
  thestruct->weighttype = 'a';
  thestruct->database = NULL;
}

int fixMatchValues( int smallmatchset, FLAGS *myflags )
{
  /* What we want to do return the appropriate value for smallmatch */
     
 if( !smallmatchset ) /* set smallmatch if it hasn't been set   */
   switch( myflags->weighttype )
     {
     case 'a':
       myflags->smallmatch = max( 0, myflags->match - 1 );
       break;
     case 'g':
       myflags->smallmatch = 1;
       break;
     case 'e':
     default:
       myflags->smallmatch = myflags->match;
       break;
     }
 if( myflags->measuretype == 'A' && !myflags->maxskips )
   myflags->maxskips = 1;
 if( myflags->debuglevel >= 1 )
   {
     printf( "fixMatchValues has been called with smallmatchset = %d\n",
	   smallmatchset );
     printf( "weighttype is %c and smallmatch is now %d\n", 
	     myflags->weighttype, myflags->smallmatch );
   }
}












