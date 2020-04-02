#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "general.h"

int max( int i, int j )
/******************************************************************************
*	returns the maximum value of the two integers			      *
*									      *
******************************************************************************/
{
  return (i>j? i: j );
}

int min( int i, int j )
/******************************************************************************
*	returns the minimum value of the two integers			      *
*									      *
******************************************************************************/
{
  return (i<j? i: j );
}


void doError( char *message, ... )
/******************************************************************************
*	sends message to stderr and exits program			      *
*									      *
******************************************************************************/
{
  va_list argptr; /* used for variable-length argument list processing */
  va_start( argptr, message ); /* get first argument */
  fprintf( stderr, message );
  va_end( argptr ); /* clean up variable-length argument list processing */
  exit(1);
}

void stripCR( char *string )
/******************************************************************************
*	Turn the trailing carriage return into a null			      *
*									      *
******************************************************************************/
{
  int crspot = strlen( string ) - 1;
  if( string[ crspot ]== '\n' )
    string[ crspot ] = '\0';
}

/******************************************************************************
*	Convert a string to lowercase					      *
*									      *
******************************************************************************/
void strToLower( char *string )
{
  while( *string )
    {
    *string = tolower( *string );
      string++;
    }
}

char *getLargeString()
/******************************************************************************
*	read a large string from standard in (longer than usual buffer size)  *
*									      *
******************************************************************************/
{
char *wordBuffer, *remstr, *str;
int crspot;
static int bigbuf = BUFLENGTH;
char *retval;

/*printf( "getLargeString called\n" );*/
if( ! (wordBuffer = (char *) malloc( sizeof( char ) * bigbuf ) ) )
     doError( "Failed to allocate space for word buffer\n" );
retval = fgets( wordBuffer, bigbuf, stdin );
crspot = strlen( wordBuffer ) - 1;
remstr = &( wordBuffer[ crspot ] );
if( retval != wordBuffer )
     return retval;
/*printf( "wordBuffer contains %s and is %d long\n", wordBuffer, 
	strlen( wordBuffer ) );*/
while( *remstr!= '\n' )
  { /* We haven't finished reading the line yet! */
    /*    printf( "Line is longer than buffer\n" );*/
    bigbuf += BUFLENGTH;
    /*    printf( "buffer size in getLargeString is %d\n", bigbuf );*/
    if( ! ( wordBuffer = 
	(char *) realloc( wordBuffer, ( sizeof( char ) * bigbuf ) ) ) )
      doError( "Failed to reallocate space for word buffer\n" );
    crspot = strlen( wordBuffer ) - 1;
    remstr = &( wordBuffer[ crspot + 1 ] ); /* redefine remstr as it has moved */
    if( !fgets( remstr, BUFLENGTH, stdin ) )
     doError( "Failed to get string from stdin\n" );
    crspot = strlen( remstr ) - 1;
    remstr = &( remstr[ crspot ] );
    /*    printf( "wordBuffer contains %s and is %d long\n", wordBuffer, 
	strlen( wordBuffer ) );*/
  }
stripCR( wordBuffer );
if( !( str = strdup( wordBuffer ) ) )
  doError( "Error duplicating string\n" );
free( wordBuffer );
/*printf( "str now is %s\n", str );*/
return str;
}


char *fgetLargeString( FILE *infile )
/******************************************************************************
*	read a large string from standard in (longer than usual buffer size)  *
*									      *
******************************************************************************/
{
char *wordBuffer, *remstr, *str;
int crspot;
static int bigbuf = BUFLENGTH;
char *retval;

/*printf( "fgetLargeString called\n" );*/
if( ! (wordBuffer = (char *) malloc( sizeof( char ) * bigbuf ) ) )
     doError( "Failed to allocate space for word buffer\n" );
retval = fgets( wordBuffer, bigbuf, infile );
crspot = strlen( wordBuffer ) - 1;
remstr = &( wordBuffer[ crspot ] );
if( retval != wordBuffer )
     return retval;
/*printf( "wordBuffer contains %s and is %d long\n", wordBuffer, 
	strlen( wordBuffer ) );*/
while( *remstr!= '\n' )
  { /* We haven't finished reading the line yet! */
    /*    printf( "Line is longer than buffer\n" );*/
    bigbuf += BUFLENGTH;
    /*    printf( "buffer size in getLargeString is %d\n", bigbuf );*/
    if( ! ( wordBuffer = 
	(char *) realloc( wordBuffer, ( sizeof( char ) * bigbuf ) ) ) )
      doError( "Failed to reallocate space for word buffer\n" );
    crspot = strlen( wordBuffer ) - 1;
    remstr = &( wordBuffer[ crspot + 1 ] ); /* redefine remstr as it has moved */
    if( !fgets( remstr, BUFLENGTH, infile ) )
     doError( "Failed to get string from input file\n" );
    crspot = strlen( remstr ) - 1;
    remstr = &( remstr[ crspot ] );
    /*    printf( "wordBuffer contains %s and is %d long\n", wordBuffer, 
	strlen( wordBuffer ) );*/
  }
stripCR( wordBuffer );
if( !( str = strdup( wordBuffer ) ) )
  doError( "Error duplicating string\n" );
free( wordBuffer );
/*printf( "str now is %s\n", str );*/
return str;
}

/******************************************************************************
*									      *
*	 Binary search in arr for word in stoplist 			      *
*									      *
******************************************************************************/
int wordSearch( char *arr[], int numElements, char *theWord )
{
  int		high = numElements-1, low = 0, k;
  int		val;

  /* printf( "Starting word search, high = %d, low = %d\n", high, low ); */
  while( low <= high )
    {
      k = low + 0.5 * 
	       ( high - low );
      	printf( "k = %d, starting strcmp to compare %s to %s\n", k, arr[k],
		theWord ); 
      val = strcmp( arr[k], theWord );
      /* 	printf( "Finished strcmp call\n" ); */
      if( val < 0 )
	low = k + 1;
      else if( 0 < val )
	high = k - 1;
      else
	return(k);
    }
  /* printf( "Finished word search\n" ); */
  return( ( val == 0 ) ? k : -1 );
}

/******************************************************************************
*									      *
*	 Allocate a new array element for an array of pointers		      *
*									      *
******************************************************************************/
char **allocArrayElement( int *storeLength, void *arrayptr, int reallocSize,
			int ptrSize, int structureSize, int pos )
{
char **newPtr = arrayptr;
if( pos >= *storeLength )
  {
    *storeLength += reallocSize;
    if( ! (newPtr = (char **) realloc( arrayptr, (ptrSize * *storeLength) ) ) )
      doError( "Failed to reallocate memory for array\n" );
  }
if( ! ( *(newPtr + pos) = (char *) malloc( structureSize ) ) )
  doError( "Failed to allocate memory for array element\n" );
return newPtr;
}

