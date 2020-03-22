/* File: melsim.c 
   Author: Sandra Uitdenbogerd
   Date: Sep 2000
   Comments: The current version has the local alignment corrected for
             all 4 local alignment functions in this file.
	     These had originally been initialised in the manner of
	     global alignment and are now fixed to initialise the first
	     row and column to 0.
	     This version includes coordinate matching and skip
	     matches.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

/*#include "top.h"*/
#include "melodies.h"
#include "general.h"
#include "melsim.h"
#include "meldb.h"

#define MATCH 1
#define MISMATCH 0
#define PENALISE 1
/* DEBUG used for once per song statements (or less */
#define DEBUG 0
/* debug is used for more frequent debug statements */
#define debug 0


int alphasize( char notetype )
{
  /* This returns the alphabet size given the note representation type.
     It is used to calculate the size of n-gram arrays and trees.
   */
switch( notetype )
  {
  case 'c': return 4;
  case 'd': return 26;
  case 'i': return 256;
  case 'C': return 12;
  case 'D': return 78;
  case 'I': return 256;
  default: return 0;
  }
}

long hamming( char *str1, char *str2 )
{
  /*
    This calculates the hamming distance (but not the minimum hamming
    distance).
   */
long ham = 0;
char *s1 = str1, *s2 = str2;

while( *s1 && *s2 )
  {
    if( *s1 != *s2 )
      {
	ham++;
      }
    s1++; s2++;
  }
if( *s1 )
  ham += strlen( s1 );
else
  ham += strlen( s2 );
return ham;
}

long minhamming( char *str1, char *str2 )
{
  /*
    This calculates the minimum hamming distance.  It uses the 
    hamming function
  */
char *s1 = str1, *s2 = str2;
long min, ham, len1, len2;

len1 = strlen( str1 );
len2 = strlen( str2 );
min =  (len1 > len2) ? len1 : len2 ;

while( *s1 )
  {
    ham = hamming( s1, s2 ) + (long)s1 - (long) str1
	  + (long)s2 - (long) str2;
    if( ham < min )
	  min = ham;
    s1++;
  }
s1 = str1;
s2++;
while( *s2 )
  {
    ham = hamming( s1, s2 ) + (long)s1 - (long) str1
	  + (long)s2 - (long) str2;
    s2++;
    if( ham < min )
      min = ham;
  }
return min;
}

int sim( char c1, char c2, int mismatch, int indel, FLAGS myflags )
{
  /*
    This calculates the score for a given comparison of characters.
    It is only used in the older dynamic programming functions in this
    module, having been superceded by the SIM macro.
  */

if( c1 == c2 )
  return myflags.match;
else if ( c1 == ' ' || c2 == ' ')
  return indel;
else
  return mismatch;
}

int simNotes( NOTE **n1p, NOTE **n2p, int mismatch, FLAGS myflags )
{
  /* notes need to be compared for interval rhythm and or stress
     We will use interval and rhythm in this function 
  */
NOTE *n1 = *n1p, *n2 = *n2p;
int intsim =  (n1->interval == n2->interval );
int rhythmsim = (n1->rhythm == n2->rhythm);
int stresssim =  (n1->stress == n2->stress);

#if debug
printf("n1 is %i %c %c\t n2 is %i %c %c\n", n1->interval, n1->rhythm,
       n1->stress, n2->interval, n2->rhythm, n2->stress);
#endif
 if( intsim && rhythmsim && stresssim )
   return myflags.match;
 else if( intsim && rhythmsim )
   return myflags.match;
 else if( rhythmsim && n1->rhythm == 'S') /* different pitch same rhythm */
   return mismatch;
 else if( intsim ) /* same pitch different rhythm */
   return myflags.match;
 else if( rhythmsim ) /* different pitch, same rhythm, not shorter */
   return mismatch;
 else
   return mismatch;
}


long localalignment( char *str1, char *str2, int mismatch, int indel,
		     FLAGS myflags )
{
  /*
    This is the first localalignment method implemented.  It uses a
    2D array of shorts and the sim function.  It uses array indices 
    instead of pointers.  I don't think I use it at the moment...
  */

short **match;
/*long **match; */
long len1 = strlen( str1 );
long len2 = strlen( str2 );
long maxlen = max( len1, len2 );
int i, j, best = 0;

#if DEBUG
printf( "Comparing string \n%s and \n%s\n", str1, str2 );
printf( "About to allocate %d squared\n", maxlen+1 );
#endif

if( maxlen >= 32767 )
  doError( "Song length too large\n" );
match = createShort2DArray( len1+1, len2+1 );
for( i=0; i <= len1; i++ )
  {
    /* Let's avoid negative array indices... */
  match[i][0] = 0;
  }
for( i=0; i <= len2; i++ )
  match[0][i] = 0;


for( i=1; i <= len2; i++ ) /* alg says to set the first row */
  match[0][i] = match[0][i-1] + sim( ' ', str2[i-1], mismatch, indel, myflags );

for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
  {
    match[i][0] = match[i-1][0] + sim(str1[i-1], ' ', mismatch, indel, myflags );
    for( j=1; j <= len2 ; j++ ) 
      {
      match[i][j] = max( 0, max( match[i-1][j] +
			 sim( str1[i-1], ' ', mismatch, indel, myflags ),
			 max( match[i][j-1] + sim(str2[j-1], ' ', 
						  mismatch, indel, myflags ),
			 match[i - 1][j - 1] + sim( str1[i-1], str2[j-1], 
						    mismatch, indel, myflags ))));
      best = max(match[i][j], best ); /* remember best so far */
      }
  }
/*showMatrix( match, maxlen, len1, len2, str1, str2 );*/
#if DEBUG
  printf("best in local alignment is %d\n",best );
#endif
freeShortArray( match, len1+1 );
return best;
}

long smalllocalalignment( char *str1, char *str2, int mismatch, int indel,
			  FLAGS myflags )
{
  /*
    This is local alignment that doesn't store the whole 2D array, only
    keeping the current and previous row.
    It is the default local alignment method for melquery.
  */
  short **match;
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = 0;

  if( myflags.debuglevel > 2 )
    {
      printf( "smalllocalalignment function\n" );
      printf( "len1 is %d, len2 is %d \n", len1, len2 );
      printf( "Comparing string \n%s and \n%s\n", str1, str2 );
      printf( "maxlen is %d\n", maxlen );
    }

  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row to 0*/
    {
      *prev++ = 0;
    }

  if( myflags.debuglevel > 3 )
    showArray( previous, min( len2+1, 40 ), "previous" );
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr = max( 0, max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + SIM( *s1, *s2, 
					 mismatch, indel, myflags.match ))));
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++;
	}
 if( myflags.debuglevel > 3 )
   {
      showArray( current, min( len2+1, 40 ), "current" );
      printf("best in localalignment so far is %d\n",best );
   }
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
    }/* end outer loop */

#if DEBUG
  showArray( current, len2+1, "current" );
  printf("best in localalignment is %d\n",best );
  printf( "len1 is now %d, len2 is %d \n", len1, len2 );
  printf( "Compared strings \n%s and \n%s\n", str1, str2 );
#endif
  return best;
}

long mixedalignment( char *str1, char *str2, int mismatch, int indel,
			  FLAGS myflags )
{
  /*
    This is a combination of local and global alignment 
    (that doesn't store the whole 2D array, only
    keeping the current and previous row).
    It uses global initialisation and the best score within the matrix.
  */
  short **match;
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = LONG_MIN;

  if( myflags.debuglevel > 2 )
    {
      printf( "mixedalignment function\n" );
      printf( "len1 is %d, len2 is %d \n", len1, len2 );
      printf( "Comparing string \n%s and \n%s\n", str1, str2 );
      printf( "maxlen is %d\n", maxlen );
    }

  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* set first row using indels */
    {
      *prev++ = *(prev-1) + indel;
    }
  /*  *current = 0; /* set first column of current */

  if( myflags.debuglevel > 3 )
    showArray( previous, min( len2+1, 40 ), "previous" );
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
     *current = *previous + indel;
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr =  max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + SIM( *s1, *s2, 
					 mismatch, indel, myflags.match )));
	  if( i == len1 )
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++;
	}
 if( myflags.debuglevel > 3 )
   {
      showArray( current, min( len2+1, 40 ), "current" );
      printf("best in mixedalignment so far is %d\n",best );
   }
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
    }/* end outer loop */

#if DEBUG
  showArray( current, len2+1, "current" );
  printf("best in mixedalignment is %d\n",best );
  printf( "len1 is now %d, len2 is %d \n", len1, len2 );
  printf( "Compared strings \n%s and \n%s\n", str1, str2 );
#endif
  return best;
}

long mixedtbalignment( char *str1, char *str2, int mismatch, int indel,
			  FLAGS myflags, MATCHINFO *matchinfo )
{
  /*
    This is a combination of local and global alignment 
    (that doesn't store the whole 2D array, only
    keeping the current and previous row).
    It uses global initialisation and the best score within the matrix.
    This version implements traceback.
  */
 short **match;
 long len1 = strlen( str1 );
 long len2 = strlen( str2 );
 long maxlen = max( len1, len2 );
 register int i, j;
 long best = LONG_MIN; 

 matchinfo->qbestpos = 0; 
 matchinfo->sbestpos = 0;


  if( myflags.debuglevel > 2 )
    {
      printf( "mixedtbalignment function\n" );
      printf( "len1 is %d, len2 is %d \n", len1, len2 );
      printf( "Comparing string \n%s and \n%s\n", str1, str2 );
      printf( "maxlen is %d\n", maxlen );
    }

if( maxlen >= 32767 )
  doError( "Song length too large\n" );
match = createShort2DArray( len1+1, len2+1 );
 match[0][0] = 0;
for( i=1; i <= len1; i++ )
  {
  match[i][0] = match[i-1][0] + indel;
  }
for( i=1; i <= len2; i++ )
  match[0][i] = match[0][i-1] + indel;


for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
  {
    for( j=1; j <= len2 ; j++ ) 
      {
       match[i][j] = max( match[i-1][j] + indel,
			 max( match[i][j-1] + indel,
			 match[i - 1][j - 1] + SIM( str1[i-1], str2[j-1], 
						    mismatch, indel, myflags.match )));
       if( match[i][j] > best )
	{
	 best = match[i][j]; /* remember best so far */
	 matchinfo->qbestpos = i-1;
	 matchinfo->sbestpos = j-1;
	}
      }
  }
 if( myflags.debuglevel > 3 )
   {
    showMatrix( match, maxlen, len1, len2, str1, str2 );
   }
freeShortArray( match, len1+1 );
 if( myflags.debuglevel > 2 )
  printf( "best score is %d, qpos %d and spos %d\n", best,
	matchinfo->qbestpos, matchinfo->sbestpos  );
return best;

}

long bizarroLocalAlignment( char *str1, char *str2, int mismatch, int indel,
			  FLAGS myflags )
{
  /*
    This is local alignment that doesn't store the whole 2D array, only
    keeping the current and previous row.
    This one increases the match weight with each adjacent match.
  */
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long previousweight[len2+1], *prevweight;
  long current[len2+1], *curr;
  long currentweight[len2+1], *currweight;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = 0;

  if( myflags.debuglevel >= 2 )
    {
      printf( "bizarrolocalalignment function\n" );
      printf( "len1 is %d, len2 is %d \n", len1, len2 );
      printf( "Comparing string \n%s and \n%s\n", str1, str2 );
      printf( "maxlen is %d\n", maxlen );
    }

  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  prev = previous;
  prevweight = previousweight;
  for( i=0; i <= len2; i++ ) /* alg says to set the first row to 0*/
    {
      *prev++ = 0;
      *prevweight++ = myflags.match - 1;
    }
  if( myflags.debuglevel >= 3 )
  {
    showArray( previous, min( len2+1, 40 ), "previous" );
    showArray( previousweight, min( len2+1, 40 ), "previousweight" );
  }
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0; /* set first column */
      *currentweight = 0; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      currweight = currentweight + 1;
      prevweight = previousweight + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  if( *s1 == *s2 ) /* match and increase match weight */
	    {
	      *currweight = *(prevweight-1) + 1;
	      *curr = *(prev-1) + *currweight;
	      if( myflags.debuglevel >= 4 )
		printf( "j=%d: %c and %c are the same, adding %d\n", j, *s1, 
			*s2, *currweight );
	    }
	  else /* mismatch and re-set match weight to zero */
	    {
	      *curr = 0;
	      *currweight = myflags.match - 1;
	    }
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++; currweight++; prevweight++;
	}
      if( myflags.debuglevel >= 3 )
	{
	  showArray( current, min( len2+1, 40 ), "current" );
	  showArray( currentweight, min( len2+1, 40 ), "currentweight" );
	  printf("best in localalignment so far is %d\n",best );
	}
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      memcpy( previousweight, currentweight, sizeof( previousweight ) );
      s1++;
    }/* end outer loop */
  if( myflags.debuglevel >= 2 )
    {
      printf("best in localalignment is %d\n",best );
      printf( "len1 is now %d, len2 is %d \n", len1, len2 );
      printf( "Compared strings \n%s and \n%s\n", str1, str2 );
    }
  return best;
}

long longestSubstring( char *str1, char *str2, int mismatch, int indel,
			  FLAGS myflags )
{
  /*
    This is local alignment that doesn't store the whole 2D array, only
    keeping the current and previous row.
    This one doesn't allow indels or mismatches and requires a minimum
    substring length of g before adding to the score.
  */
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long previouslength[len2+1], *prevlength;
  long current[len2+1], *curr;
  long currentlength[len2+1], *currlength;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = 0;

  if( myflags.debuglevel >= 2 )
    {
      printf( "longestSubstring function\n" );
      printf( "len1 is %d, len2 is %d \n", len1, len2 );
      printf( "Comparing string \n%s and \n%s\n", str1, str2 );
      printf( "maxlen is %d and min substringlength is %d\n", maxlen,
	      myflags.ngramsize );
    }

  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  prev = previous;
  prevlength = previouslength;
  for( i=0; i <= len2; i++ ) /* alg says to set the first row to 0*/
    {
      *prev++ = 0;
      *prevlength++ = 0;
    }
  if( myflags.debuglevel >= 3 )
  {
    showArray( previous, min( len2+1, 40 ), "previous" );
    showArray( previouslength, min( len2+1, 40 ), "previouslength" );
  }
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0; /* set first column */
      *currentlength = 0; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      currlength = currentlength + 1;
      prevlength = previouslength + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  if( *s1 == *s2 ) /* match and increase length */
	    {
	      *currlength = *(prevlength-1) + 1;
	      if( *currlength >= myflags.ngramsize )
		*curr = *(prev-1) + myflags.match;
	      else
		*curr = 0;
	      if( myflags.debuglevel >= 4 )
		printf( "j=%d: %c and %c are the same, adding %d\n", j, *s1, 
			*s2, *currlength );
	    }
	  else /* mismatch and re-set match weight to zero */
	    {
	      *curr = 0;
	      *currlength = 0;
	    }
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++; currlength++;prevlength++;
	}
      if( myflags.debuglevel >= 3 )
	{
	  showArray( current, min( len2+1, 40 ), "current" );
	  showArray( currentlength, min( len2+1, 40 ), "currentlength" );
	  printf("best in localalignment so far is %d\n",best );
	}
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      memcpy( previouslength, currentlength, sizeof( previouslength ) );
      s1++;
    }/* end outer loop */
  if( myflags.debuglevel >= 2 )
    {
      printf("best in localalignment is %d\n",best );
      printf( "len1 is now %d, len2 is %d \n", len1, len2 );
      printf( "Compared strings \n%s and \n%s\n", str1, str2 );
    }
  return best;
}

long longestSubstringWithGaps( char *str1, char *str2, int mismatch, int indel,
			  FLAGS myflags )
{
  /*
    This is local alignment that doesn't store the whole 2D array, only
    keeping the current and previous row.
    This one doesn't allow indels or mismatches and requires a minimum
    substring length of g before adding to the score.
  */
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long previouslength[len2+1], *prevlength;
  long current[len2+1], *curr;
  long currentlength[len2+1], *currlength;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = 0;

  if( myflags.debuglevel >= 2 )
    {
      printf( "longestSubstringWithGaps function\n" );
      printf( "len1 is %d, len2 is %d \n", len1, len2 );
      printf( "Comparing string \n%s and \n%s\n", str1, str2 );
      printf( "maxlen is %d and min substringlength is %d\n", maxlen,
	      myflags.ngramsize );
    }

  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  prev = previous;
  prevlength = previouslength;
  for( i=0; i <= len2; i++ ) /* alg says to set the first row to 0*/
    {
      *prev++ = 0;
      *prevlength++ = 0;
    }
  if( myflags.debuglevel >= 3 )
  {
    showArray( previous, min( len2+1, 40 ), "previous" );
    showArray( previouslength, min( len2+1, 40 ), "previouslength" );
  }
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0; /* set first column */
      *currentlength = 0; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      currlength = currentlength + 1;
      prevlength = previouslength + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  if( *s1 == *s2 ) /* match and increase length */
	    {
	      *currlength = *(prevlength-1) + 1;
	      if( *currlength == myflags.ngramsize ) /* add to previous best */
		{
		  if( best )
		    *curr = best + myflags.smallmatch;
		  else
		    *curr = *(prev-1) + myflags.match; /*is prev-1 ever > 0? */
		}
	      else if( *currlength > myflags.ngramsize )
		*curr = *(prev-1) + myflags.match;
	      else
		*curr = 0;
	      if( myflags.debuglevel >= 4 )
		printf( "j=%d: %c and %c are the same, adding %d\n", j, *s1, 
			*s2, *currlength );
	    }
	  else /* mismatch and re-set match weight to zero */
	    {
	      *curr = 0;
	      *currlength = 0;
	    }
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++; currlength++;prevlength++;
	}
      if( myflags.debuglevel >= 3 )
	{
	  showArray( current, min( len2+1, 40 ), "current" );
	  showArray( currentlength, min( len2+1, 40 ), "currentlength" );
	  printf("best in localalignment so far is %d\n",best );
	}
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      memcpy( previouslength, currentlength, sizeof( previouslength ) );
      s1++;
    }/* end outer loop */
  if( myflags.debuglevel >= 2 )
    {
      printf("best in localalignment is %d\n",best );
      printf( "len1 is now %d, len2 is %d \n", len1, len2 );
      printf( "Compared strings \n%s and \n%s\n", str1, str2 );
    }
  return best;
}

long bizarroSkipLocalAlignment( char *str1, char *str2, int mismatch, int indel,
			  FLAGS myflags )
{
  /*
    This is local alignment that doesn't store the whole 2D array, only
    keeping the current and previous row.
    This one increases the match weight with each adjacent match but
    also allows a single skip match in the song being matched to the
    query.
  */
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long previousweight[len2+1], *prevweight;
  long current[len2+1], *curr;
  long currentweight[len2+1], *currweight;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = 0;

  if( myflags.notetype != 'd' )
    doError( "This option is only implemented for mod 12 intervals\n" );

  if( myflags.debuglevel >= 2 )
    {
      printf( "bizarrolocalalignment function\n" );
      printf( "len1 is %d, len2 is %d \n", len1, len2 );
      printf( "Comparing string \n%s and \n%s\n", str1, str2 );
      printf( "maxlen is %d\n", maxlen );
    }

  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  prev = previous;
  prevweight = previousweight;
  for( i=0; i <= len2; i++ ) /* alg says to set the first row to 0*/
    {
      *prev++ = 0;
      *prevweight++ = myflags.match - 1;
    }
  if( myflags.debuglevel >= 3 )
  {
    showArray( previous, min( len2+1, 40 ), "previous" );
    showArray( previousweight, min( len2+1, 40 ), "previousweight" );
  }
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0; /* set first column */
      *currentweight = 0; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      currweight = currentweight + 1;
      prevweight = previousweight + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  if( *s1 == *s2 ) /* match and increase match weight */
	    {
	      *currweight = *(prevweight-1) + 1;
	      *curr = *(prev-1) + *currweight;
	      if( myflags.debuglevel >= 4 )
		printf( "j=%d: %c and %c are the same, adding %d\n", j, *s1, 
			*s2, *currweight );
	    }
	  else if( j > 1 && *s1 == *s2 + *(s2-1) - 'm' )
	    {
	      /* Use the same weight and don't increment */
	      *curr = *(prev-1) + *currweight;
	      if( myflags.debuglevel >= 4 )
		printf( "j=%d: skip match, adding %d\n", j, 
			*currweight );	      
	    }
	  else /* mismatch and re-set match weight to zero */
	    {
	      *curr = 0;
	      *currweight = myflags.match - 1;
	    }
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++; currweight++; prevweight++;
	}
      if( myflags.debuglevel >= 3 )
	{
	  showArray( current, min( len2+1, 40 ), "current" );
	  showArray( currentweight, min( len2+1, 40 ), "currentweight" );
	  printf("best in localalignment so far is %d\n",best );
	}
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      memcpy( previousweight, currentweight, sizeof( previousweight ) );
      s1++;
    }/* end outer loop */
  if( myflags.debuglevel >= 2 )
    {
      printf("best in localalignment is %d\n",best );
      printf( "len1 is now %d, len2 is %d \n", len1, len2 );
      printf( "Compared strings \n%s and \n%s\n", str1, str2 );
    }
  return best;
}

long noIndelLocalalignment( char *str1, char *str2, int mismatch, int indel,
			  FLAGS myflags )
{
  /*
    This is local alignment that doesn't store the whole 2D array, only
    keeping the current and previous row.
    This version doesn't consider indels in matching.
  */
  short **match;
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = 0;

  if( myflags.debuglevel >= 2 )
    {
      printf( "noIndelLocalalignment function\n" );
      printf( "len1 is %d, len2 is %d \n", len1, len2 );
      printf( "Comparing string \n%s and \n%s\n", str1, str2 );
      printf( "maxlen is %d\n", maxlen );
    }


  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row to 0*/
    {
      *prev++ = 0;
    }

#if debug
  showArray( previous, min( len2+1, 40 ), "previous" );
#endif
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  if( myflags.measuretype == 'M' ) /* No mismatches or indels */
	    *curr = MAX( 0,  ( *s1==*s2 ? *(prev-1) + myflags.match: 0 ) );
	  else
	    *curr = MAX( 0,  (*(prev-1) + SIM( *s1, *s2, 
		     mismatch, indel, myflags.match )));
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++;
	}
#if debug
      showArray( current, min( len2+1, 40 ), "current" );
      printf("best in localalignment so far is %d\n",best );
#endif
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
    }/* end outer loop */

#if DEBUG
  showArray( current, len2+1, "current" );
  printf("best in localalignment is %d\n",best );
  printf( "len1 is now %d, len2 is %d \n", len1, len2 );
  printf( "Compared strings \n%s and \n%s\n", str1, str2 );
#endif
  return best;
}

long ** getExtraRows( long len, int extras, long *previous, FLAGS myflags )
{
  /*
    If using more than one skip we need to refer to other rows of the
    matrix.  So this allocates memory for them.  We are assuming that
    the matrix is a lot larger than the number of skips permitted.
  */
 long **skiprows, i;
 long size = sizeof( long ) * len ;

  /* debuglevel should be 2 */
  if( myflags.debuglevel >= 1 )
    printf( "getExtraRows called with length %d\n", len );
  if( !( skiprows = (long **)  
	 malloc( sizeof( long *) * extras ) ) )
    doError( "Failed to allocate extra skip rows\n" );
  for( i=0; i < extras; i++ )
    {
      if( !( skiprows[i] = (long *) malloc( size ) ) )
	doError( "Failed to allocate extra skip rows\n" );
      memcpy( skiprows[i], previous, size );
    }
  return skiprows;
}

void updateExtraRows( long **skiprows, int extras, long len, long *previous, 
		      FLAGS myflags )
{
  register int i;
  long size = sizeof( long ) * len ;

  /* debuglevel should be 2 */
  if( myflags.debuglevel >= 1 )
    printf( "updateExtraRows called with length %d\n", len );
  for( i=1; i <= extras-1; i++ )
    memcpy( skiprows[i-1], skiprows[i], size );
  memcpy( skiprows[extras-1], previous, size );

}


long **freeExtraRows( long **skiprows, FLAGS myflags )
{
  int i;
  /* debuglevel should be 3 */
  if( myflags.debuglevel >= 1 )
    printf( "freeExtraRows called\n" );
  for( i=0; i < MAX( (myflags.maxskips - 1), myflags.queryskips ); i++ )
    free( skiprows[i] );
  free( skiprows );
}

long skipValue( int skipcount, FLAGS myflags )
{
  /* This returns the value to be used in the DP algorithm
     given the number of skips 
  */
  double ratio;

  switch( myflags.weighttype )
    {
    case 'a':
      return max( myflags.smallmatch - skipcount + 1, 0 );
      break;
    case 'g':
      ratio = myflags.smallmatch / myflags.match ;
      return (long) max( pow( 
	      ratio, myflags.maxskips - skipcount - 1) * myflags.smallmatch , 
			 1 );
      break;
    case 'e':
    default:
      return myflags.smallmatch;
      break;
    }
}


long specLocalAlignment( char *str1, char *str2, int mismatch, int indel,
			 FLAGS myflags )
{
  /* This version will calculate the score with an allowance for 
     calculated skipped intervals.  The current version only allows
     one skip in the song (str2) and none in the query (str1).

     It is currently being extended to allow a max of 2 skips in
     either query or song.
     Assumptions: myflags.maxskips >= 0
  */
  short **match;
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j; /* i is used for the query (s1) and j for the song (s2) */
  long best = 0;
  long **skiprows;
  int extras = MAX( (myflags.maxskips - 1), 
		    myflags.queryskips * myflags.maxskips );
  int extrascore = 0;
  int skipcount, skipPos;

  if( myflags.debuglevel >= 2 )
    {
      printf( "specLocalAlignment called with: " );
      printf( "len1 = %d, len2 = %d \n", len1, len2 );
      printf( "Comparing string \n%s and \n%s\n", str1, str2 );
      printf( "maxlen is %d\n", maxlen );
      printf( "extras = %d\n", extras );
    }
  if( myflags.notetype != 'd' )
    doError(
	"Error: Special local alignment only implemented for modulo-12\n" );
  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row to 0*/
      *prev++ = 0;
  if( extras )
      skiprows = getExtraRows( (len2+1), extras, previous, myflags );
#if debug
  showArray( previous, min( len2+1, 40 ), "previous" );
#endif
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  /* if we are doing extra skips, let's calculate that first */
	  if( extras )
	    {
	      extrascore = 0;
	      if( myflags.queryskips ) 
		{
		  /* extrascore contains the value that has been calculated
		     by using skips in the query
		  */
		      /* calculate score for single query skip */
		  extrascore = max( *(skiprows[ extras - 1 ] + j - 1) + 
			SIM( (*( s1 - 1 ) + *s1 - 'm'), *s2, mismatch, indel, 
			 skipValue( 1, myflags ) ) , extrascore );
		  if( myflags.debuglevel >= 3)
		    printf( "skip value is %d, extrascore is %d, j=%d\n", 
			    skipValue( 1, myflags ), extrascore, j );
		  if( myflags.maxskips > 1 )
		    {
		      /* calculate score for query + song skip */
		      extrascore = 
			max( extrascore, max( 
			     ( *s1 + *(s1-1) == *s2 + *(s2-1) )?
			     skipValue( 2, myflags ) 
			     + *(skiprows[ extras-1] + j - 2 ):0,
			     ( *s1 + *(s1-1) + *(s1-2) - 2 * 'm'
			       == *s2 )? skipValue( 2, myflags )
			     + *( skiprows[ extras-2] + j - 1):0));
	     
		    }
		}
	      if( myflags.debuglevel >= 4 )
		printf( "special extras, extrascore = %d\n", extrascore );
	    }
	  /* compare 0, 
	     previous row (prev) , previous column (curr-1) for indels,
	     cell in previous row and col (prev-1) for match/mismatch and 
	     summed intervals from skip position for match */
	  if( j > 2 && myflags.maxskips >= 2 )
	    {
	      extrascore = max( extrascore, 
			      (*s1 == *(s2-2) + *(s2-1) + *s2 - 2*'m')?
			      *(prev-3) + skipValue( 2, myflags ):0); 
	      if( myflags.debuglevel >= 3)
		printf( "j = %d, skip value is %d, extrascore is %d\n", 
			j, skipValue( 2, myflags ), extrascore );
	    }
	  if( j > 1 )  /* can't compare skip on first column */
	    {
	      *curr = max( 0, max( *prev + indel, 
		  max( *(curr-1) + indel,
		  max( *(prev-1) + SIM( *s1, *s2, mismatch, indel, myflags.match ),
		  max( ( *s1 == (*( s2-1 ) + *s2  - 'm')?
				*(prev-2) + myflags.smallmatch : 0 ),
		       extrascore )))));
	      if( myflags.debuglevel >= 3 )
		{
		  printf( "prev+indel %d, curr-1 + indel %d, prev-1+sim %d, prev-2+sim %d, extrascore %d\n", 
			  *prev + indel, *(curr-1) + indel,
			*(prev-1) + SIM( *s1, *s2, mismatch, indel, myflags.match ),
			*(prev-2) + SIM( *s1, (*( s2-1 ) + *s2 - 'm' - 'm'), mismatch,
					 indel, myflags.smallmatch ),
			  extrascore );
		  printf( "prev-2 %d, sim %d\n", *(prev-2),
			  SIM( *s1, (*( s2-1 ) + *s2 - 'm' - 'm'), mismatch,
			       indel, myflags.smallmatch ) );
		  printf( "*s1= %c, *s2 = %c\n", *s1, *s2 );
		}
	    }
	  else
	    *curr = max( 0, max( *prev + indel, max( extrascore,
		    max( *(curr-1) + indel,
		   *(prev-1) + SIM( *s1, *s2, mismatch, indel, 
				    myflags.match )))) );
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++;
	}
#if debug
      if( myflags.debuglevel >= 4 && extras )
	{
	  showArray( skiprows[extras-1], min( len2+1, 40 ), 
		   "skiprow[extras-1]" );
	  if( extras > 1 )
	    showArray( skiprows[extras-2], min( len2+1, 40 ), 
		   "skiprow[extras-2]" );
	}
      showArray( previous, min( len2+1, 40 ), "previous" );
      showArray( current, min( len2+1, 40 ), "current" );
      printf("best in local alignment so far is %d\n",best );
#endif
      /* update any extra stored rows */
      if( extras )
	updateExtraRows( skiprows, extras, (len2+1), previous, myflags );
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
    }/* end outer loop */

if( myflags.debuglevel >= 2 )
  {
    printf("best in local alignment is %d\n",best );
    printf( "len1 is now %d, len2 is %d \n", len1, len2 );
    printf( "Compared strings \n%s and \n%s\n", str1, str2 );
  }
if( myflags.debuglevel >= 3 )
    showArray( current, len2+1, "current" );
 if( extras )
   freeExtraRows( skiprows, myflags );
  return best;
}

long bigSkipLocalAlignment( char *str1, char *str2, int mismatch, int indel,
		     FLAGS myflags )
{
  /*
    This is a local alignment method that allows large skips.  It uses a
    2D array of shorts.  It uses array indices 
    instead of pointers.
  */

short **match;
/*long **match; */
long len1 = strlen( str1 );
long len2 = strlen( str2 );
long maxlen = max( len1, len2 );
int i, j, best = 0;

#if DEBUG
printf( "Comparing string \n%s and \n%s\n", str1, str2 );
printf( "About to allocate %d squared\n", maxlen+1 );
#endif

if( maxlen >= 32767 )
  doError( "Song length too large\n" );
match = createShort2DArray( len1+1, len2+1 );
for( i=0; i <= len1; i++ )
  {
    /* Let's avoid negative array indices... */
  match[i][0] = 0;
  }
for( i=0; i <= len2; i++ )
  match[0][i] = 0;


for( i=1; i <= len2; i++ ) /* alg says to set the first row */
  match[0][i] = match[0][i-1] + sim( ' ', str2[i-1], mismatch, indel, myflags );

for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
  {
    match[i][0] = match[i-1][0] + sim(str1[i-1], ' ', mismatch, indel, myflags );
    for( j=1; j <= len2 ; j++ ) 
      {
      match[i][j] = max( 0, max( match[i-1][j] +
			 sim( str1[i-1], ' ', mismatch, indel, myflags ),
			 max( match[i][j-1] + sim(str2[j-1], ' ', 
						  mismatch, indel, myflags ),
			 match[i - 1][j - 1] + sim( str1[i-1], str2[j-1], 
						    mismatch, indel, myflags ))));
      best = max(match[i][j], best ); /* remember best so far */
      }
  }
/*showMatrix( match, maxlen, len1, len2, str1, str2 );*/
#if DEBUG
  printf("best in local alignment is %d\n",best );
#endif
freeShortArray( match, len1+1 );
return best;
}

long specialLocalalignment( void *str1, void *str2, 
			    int len1, int len2, 
			    int (*ssim)( void *, void *, int ),
			    int elementSize,
			    int mismatch, int indel, FLAGS myflags )
{
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  void *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = 0;

#if DEBUG
  printf( "specialLocalalignment function\n" );
  printf( "len1 is %d, len2 is %d \n", len1, len2 );
  printf( "Comparing locations \n%d and \n%d\n", str1, str2 );
  printf( "elementSize is %d\n", elementSize );
  printf( "maxlen is %d\n", maxlen );
#endif

  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row to 0 */
    {
      *prev++ = 0;
    }

#if debug
  showArray( previous, min( len2+1, 40 ), "previous" );
#endif
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr = max( 0, max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + ssim( s1, s2, 
						      mismatch ))));
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; 
	  s2 += elementSize;
	  prev++;
	}
#if debug
      showArray( current, min( len2+1, 40 ), "current" );
      printf("best in local alignment so far is %d\n",best );
#endif
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1 += elementSize;
    }/* end outer loop */

#if DEBUG
  showArray( current, len2+1, "current" );
  printf("best in local alignment is %d\n",best );
  printf( "len1 is now %d, len2 is %d \n", len1, len2 );
  printf( "Compared strings \n%d and \n%d\n", str1, str2 );
#endif
  return best;
}

long globalalignment( char *str1, char *str2, int mismatch, int indel,
		      FLAGS myflags )
{
  short **match;
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = LONG_MIN;

#if DEBUG
  printf( "globalalignment function\n" );
  printf( "len1 is %d, len2 is %d \n", len1, len2 );
  printf( "Comparing string \n%s and \n%s\n", str1, str2 );
  printf( "maxlen is %d\n", maxlen );
#endif

  if( maxlen >= 32767 )
    doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row */
    {
#if PENALISE
      *prev++ = *(prev-1) + indel;
#else
     *prev++ = 0; 
#endif
    }

#if debug
  showArray( previous, min( len2+1, 40 ), "previous" );
#endif
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
#if PENALISE
      *current = *previous + indel; /* set first column */
#else
      *current = 0;
#endif
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr = max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + SIM( *s1, *s2, 
				     mismatch, indel, myflags.match )));
	  curr++; s2++;prev++;
	}
 if( myflags.debuglevel > 3 )
   {
      showArray( current, min( len2+1, 40 ), "current" );
   }
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
    }/* end outer loop */

 if( myflags.debuglevel > 3 )
   {
  showArray( current, len2+1, "current" );
  printf("global alignment is %d\n", *(curr-1));
  printf( "len1 is now %d, len2 is %d \n", len1, len2 );
  printf( "Compared strings \n%s and \n%s\n", str1, str2 );
   }
  return *(curr-1);
}

long hybridalignment( char *str1, char *str2, int mismatch, int indel,
		      FLAGS myflags )
     /** This function does global alignment with initial column and
	 row set to 0
     **/
{
  short **match;
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = 0;

  if( myflags.debuglevel > 2 )
   {
    printf( "hybridalignment function\n" );
    printf( "len1 is %d, len2 is %d \n", len1, len2 );
    printf( "Comparing string \n%s and \n%s\n", str1, str2 );
    printf( "maxlen is %d\n", maxlen );
   }
  if( maxlen >= 32767 )
   doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row */
    {
     *prev++ = 0; 
    }

 if( myflags.debuglevel > 3 )
   {
    showArray( previous, min( len2+1, 40 ), "previous" );
   }
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0;
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr = max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + SIM( *s1, *s2, 
				     mismatch, indel, myflags.match )));
	  curr++; s2++;prev++;
	}
 if( myflags.debuglevel > 3 )
   {
      showArray( current, min( len2+1, 40 ), "current" );
   }
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
    }/* end outer loop */

 if( myflags.debuglevel > 3 )
   {
  showArray( current, len2+1, "current" );
  printf("hybrid alignment is %d\n", *(curr-1));
  printf( "len1 is now %d, len2 is %d \n", len1, len2 );
  printf( "Compared strings \n%s and \n%s\n", str1, str2 );
   }
  return *(curr-1);
}

long endspacefreealignment( char *str1, char *str2, int mismatch, int indel,
		      FLAGS myflags )
     /** This function does global alignment with initial column and
	 row set to 0, then returns the max in final row/column
	 This is referred to in Gusfield p228
     **/
{
  short **match;
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = 0;

  if( myflags.debuglevel > 2 )
   {
    printf( "endspacefreealignment function\n" );
    printf( "len1 is %d, len2 is %d \n", len1, len2 );
    printf( "Comparing string \n%s and \n%s\n", str1, str2 );
    printf( "maxlen is %d\n", maxlen );
   }
  if( maxlen >= 32767 )
   doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row */
    {
     *prev++ = 0; 
    }

 if( myflags.debuglevel > 3 )
   {
    showArray( previous, min( len2+1, 40 ), "previous" );
   }
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0;
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr = max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + SIM( *s1, *s2, 
				     mismatch, indel, myflags.match )));
	  curr++; s2++;prev++;
	}
      if( myflags.debuglevel > 3 )
       {
	showArray( current, min( len2+1, 40 ), "current" );
       }
      best = max( best, *(curr-1) );
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
    }/* end outer loop */
  curr = current + 1; /* check final row for possible max */
  for( j=1; j<=len2; j++ )
   {
    best = max( best, *curr++ );
   }
 if( myflags.debuglevel > 3 )
   {
  showArray( current, len2+1, "current" );
  printf("hybrid alignment is %d\n", *(curr-1));
  printf( "len1 is now %d, len2 is %d \n", len1, len2 );
  printf( "Compared strings \n%s and \n%s\n", str1, str2 );
   }
  return best;
}

long fullqueryalignment( char *str1, char *str2, int mismatch, int indel,
		      FLAGS myflags )
     /** This function does global query alignment with initial column and
	 row set to 0, then returns the max in final row
     **/
{
  short **match;
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = LONG_MIN;

  if( myflags.debuglevel > 2 )
   {
    printf( "fullqueryalignment function\n" );
    printf( "len1 is %d, len2 is %d \n", len1, len2 );
    printf( "Comparing string \n%s and \n%s\n", str1, str2 );
    printf( "maxlen is %d\n", maxlen );
   }
  if( maxlen >= 32767 )
   doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row */
    {
     *prev++ = 0; 
    }

 if( myflags.debuglevel > 3 )
   {
    showArray( previous, min( len2+1, 40 ), "previous" );
   }
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = 0;
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr = max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + SIM( *s1, *s2, 
				     mismatch, indel, myflags.match )));
	  curr++; s2++;prev++;
	}
      if( myflags.debuglevel > 3 )
       {
	showArray( current, min( len2+1, 40 ), "current" );
       }
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
    }/* end outer loop */
  curr = current + 1; /* check final row for possible max */
  for( j=1; j<=len2; j++ )
   {
    best = max( best, *curr++ );
   }
 if( myflags.debuglevel > 3 )
   {
    showArray( current, len2+1, "current" );
    printf("fullquery alignment is %d\n", best);
    printf( "len1 is now %d, len2 is %d \n", len1, len2 );
    printf( "Compared strings \n%s and \n%s\n", str1, str2 );
   }
  return best;
}

long globalqueryalignment( char *str1, char *str2, int mismatch, int indel,
		      FLAGS myflags )
     /** This function does global query alignment with initial column to 0
	 and row initialised as for global alignment, 
	 then returns the max in final row
     **/
{
  short **match;
  long len1 = strlen( str1 );
  long len2 = strlen( str2 );
  long previous[len2+1], *prev;
  long current[len2+1], *curr;
  char *s1 = str1, *s2 = str2;
  long maxlen = max( len1, len2 );
  register int i, j;
  long best = LONG_MIN;

  if( myflags.debuglevel > 2 )
   {
    printf( "fullqueryalignment function\n" );
    printf( "len1 is %d, len2 is %d \n", len1, len2 );
    printf( "Comparing string \n%s and \n%s\n", str1, str2 );
    printf( "maxlen is %d\n", maxlen );
   }
  if( maxlen >= 32767 )
   doError( "Song length too large\n" );
  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row */
    {
     *prev++ = 0; 
    }

 if( myflags.debuglevel > 3 )
   {
    showArray( previous, min( len2+1, 40 ), "previous" );
   }
  for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
    {
      *current = *previous + indel;
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr = max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + SIM( *s1, *s2, 
				     mismatch, indel, myflags.match )));
	  curr++; s2++;prev++;
	}
      if( myflags.debuglevel > 3 )
       {
	showArray( current, min( len2+1, 40 ), "current" );
       }
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
    }/* end outer loop */
  curr = current + 1; /* check final row for possible max */
  for( j=1; j<=len2; j++ )
   {
    best = max( best, *curr++ );
   }
 if( myflags.debuglevel > 3 )
   {
    showArray( current, len2+1, "current" );
    printf("fullquery alignment is %d\n", best);
    printf( "len1 is now %d, len2 is %d \n", len1, len2 );
    printf( "Compared strings \n%s and \n%s\n", str1, str2 );
   }
  return best;
}

void showArray( int *array, int max, char *name )
{
int i;
printf( "Contents of array %s\n", name );
for( i=0; i < max; i++ )
  printf( "%4i", array[i] );
printf( "\n" );
}

void showShortArray( short *array, int max, char *name )
{
int i;
printf( "Contents of array %s\n", name );
for( i=0; i < max; i++ )
  printf( "%3i", array[i] );
printf( "\n" );
}


long uCharlocalalignment( unsigned char *str1, unsigned char *str2, 
			  int len1, int len2, int mismatch, int indel,
			  FLAGS myflags )
{
short previous[len2+1];
short current[len2+1];
short *curr = current, *prev = previous;
long maxlen = max( len1, len2 );
int i, j, best = 0;
char *s1 = str1, *s2 = str2;

#if DEBUG
printf( "len1 is %d, len2 is %d \n", len1, len2 );
printf( "maxlen is %d\n", maxlen );
#endif

if( maxlen >= 32767 )
  doError( "Song length too large\n" );

  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row to 0*/
    {
      *prev++ = 0;
    }
for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
  {

      *current = 0; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr = max( 0, max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + SIM( *s1, *s2, 
				       mismatch, indel, myflags.match ))));
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++;
	}
#if debug
      showArray( current, min( len2+1, 40 ), "current" );
      printf("best in localalignment so far is %d\n",best );
#endif
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
  }

#if DEBUG
  printf("best in localalignment is %d\n",best );
#endif
return best;
}

long specUCharLocalAlignment( unsigned char *str1, unsigned char *str2, 
			  int len1, int len2, int mismatch, int indel,
			      FLAGS myflags )
{
short previous[len2+1];
short current[len2+1];
short *curr = current, *prev = previous;
long maxlen = max( len1, len2 );
int i, j, best = 0;
char *s1 = str1, *s2 = str2;


printf( "This isn't currently implemented\n" );
#if DEBUG
printf( "len1 is %d, len2 is %d \n", len1, len2 );
printf( "maxlen is %d\n", maxlen );
#endif

if( maxlen >= 32767 )
  doError( "Song length too large\n" );

  *previous = 0;
  prev = previous + 1;
  for( i=1; i <= len2; i++ ) /* alg says to set the first row */
    {
      *prev++ = *(prev-1) + indel;
    }
for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
  {

      *current = *previous + indel; /* set first column */
      curr = current + 1;
      prev = previous + 1;
      s2 = str2;
      for( j=1; j <= len2 ; j++ ) 
	{
	  *curr = max( 0, max( *prev + indel, 
			    max( *(curr-1) + indel,
				 *(prev-1) + SIM( *s1, *s2, 
					 mismatch, indel, myflags.match ))));
	  best = MAX(*curr, best ); /* remember best so far */
	  curr++; s2++;prev++;
	}
#if debug
      showArray( current, min( len2+1, 40 ), "current" );
      printf("best in localalignment so far is %d\n",best );
#endif
      /*  set previous row equal to current row  */
      memcpy( previous, current, sizeof( previous ) );
      s1++;
  }

#if DEBUG
  printf("best in localalignment is %d\n",best );
#endif
return best;
}

long smallLCS( char *str1, char *str2, FLAGS myflags )
     /* This counts the length of the longest common subsequence */
{
short **match;
long len1 = strlen( str1 );
long len2 = strlen( str2 );
short previous[len2+1];
short current[len2+1];

long maxlen = max( len1, len2 );
int i, j, best = 0;

 if( myflags.debuglevel > 1 )
   {
     printf( "len1 is %d, len2 is %d \n", len1, len2 );
     printf( "Comparing string \n%s and \n%s\n", str1, str2 );
     printf( "maxlen is %d\n", maxlen );
   }

if( maxlen >= 32767 )
  doError( "Song length too large\n" );

for( i=0; i <= len2; i++ ) /* alg says to set the first row */
  previous[i] =  0;

for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
  {
    current[0] =  0;
    for( j=1; j <= len2 ; j++ ) 
      {
	if( str1[i-1] == str2[j-1] )
	  {
	    current[j] = previous[j-1] + 1;
	  }
	else
	  current[j] =  max( previous[j], current[j-1] );
	best = max(current[j], best ); /* remember best so far */
      }
 if( myflags.debuglevel > 3 )
   {
      showShortArray( current, min( len2+1, 40 ), "current" );
      printf("best in localalignment so far is %d\n",best );
   }

    for( j=0; j <=len2; j++ )/* set previous row equal to current row */
      previous[j] = current[j];
  }

#if DEBUG
  printf("longest common subsequence value is %d\n",best );
#endif
return best;
}

long uCharLCS( unsigned char *str1, unsigned char *str2, int len1, int len2 )
     /* This counts the length of the longest common subsequence using
      * arrays of ints */
{
short previous[len2+1];
short current[len2+1];

long maxlen = max( len1, len2 );
int i, j, best = 0;

#if DEBUG
printf( "len1 is %d, len2 is %d \n", len1, len2 );
printf( "Comparing string \n%s and \n%s\n", str1, str2 );
printf( "maxlen is %d\n", maxlen );
#endif

if( maxlen >= 32767 )
  doError( "Song length too large\n" );

for( i=0; i <= len2; i++ ) /* alg says to set the first row */
  previous[i] =  0;

for( i=1; i <= len1; i++ ) /* perform body of alignment alg */
  {
    current[0] =  0;
    for( j=1; j <= len2 ; j++ ) 
      {
	if( str1[i-1] == str2[j-1] )
	  {
	    current[j] = previous[j-1] + 1;
	  }
	else
	  current[j] =  max( previous[j], current[j-1] );
	best = max(current[j], best ); /* remember best so far */
      }
    for( j=0; j <=len2; j++ )/* set previous row equal to current row */
      previous[j] = current[j];
  }
#if DEBUG
  printf("longest common subsequence value is %d\n",best );
#endif
return best;
}

int specLCS( unsigned char *querystring, unsigned char *songstring, 
	     int qlen, int slen, short **querygrid, short **songgrid )
{
  /* This performs an interval "longest common subsequence" calculation.
     It doesn't use dynamic programming as that doesn't work.  (Maybe
     later I can figure out a way of applying it...)
  */

  int i, j, srow, scol;
  int best = 0, score;
  int *currmatchspots; /* [2][qlen] qi, si */
  int *bestmatchspots = NULL; /* [2][qlen] */

#if DEBUG
  printf("specLCS called with qlen = %d, slen = %d\n", qlen, slen );
#endif

if( !(currmatchspots = (int *) malloc( sizeof( int ) * 4 * qlen ) ) )
  doError( "Couldn't allocate currmatchspots in specLCS" );
for( i=0; i < qlen; i++ )
  for( j=i; j >= 0; j-- )
    {
      /* start a match from this point against each possible location in the 
       song.
      */
      for( srow=0; srow < slen; srow++ )
	for( scol= srow; scol >= 0; scol-- )
	  if( querygrid[i][j] == songgrid[srow][scol] )
	    {
	      *( currmatchspots ) = MAX( i, j );
	      *( currmatchspots + 1 ) = MAX( srow, scol);
	      score = startSpecLCSMatch( querystring, songstring, qlen, slen,
			       querygrid, songgrid, i, j, srow, scol, currmatchspots );
	      best = MAX( score, best );
	      if( best == score )
		{
		  if( bestmatchspots )
		    free( bestmatchspots );
		  bestmatchspots = currmatchspots;
		  if( !(currmatchspots = (int *) 
			malloc( sizeof( int ) * 2 * qlen ) ) )
		    doError( "Couldn't allocate currmatchspots in specLCS" );
		}
	    }
    }
showMatchSpots( qlen, slen, querygrid,
		 songgrid, bestmatchspots, best );
return best;
}

void printSpaces( int n )
{
int i;
for( i= 0; i < n; i++ )
  putchar( ' ' );
}

void showMatchSpots( int qlen, int slen, short **querygrid, short **songgrid,
	     int *matchspots, int best )
{
  /* This displays the two strings and their matched characters 
     Note: best contains the number of matches stored in matchspots 
  */
int i, diff, sdiff, thispos, lastpos, thisspos, lastspos;

lastpos = -1;
lastspos = -1;
 for( i=0; i < qlen; i++ ) /* write out the query string */
  {
    printf( "%4d", querygrid[i][i] );
  }
 putchar('\n');
 for( i=0; i < best; i++ ) /* write out the match points */
  {
    thispos =  *(matchspots + 2*i );
#if debug
    printf( "in vertical bar loop, i = %d, thispos = %d\n", i, thispos );
#endif
    printSpaces( (thispos - lastpos - 1 ) * 4 + 3 );
    putchar( '|' );
    lastpos = thispos;
  }
 putchar('\n');
 for( i=0; i < best; i++ ) /* write out the match points */
  {
    thisspos = *( matchspots + 2*i + 1 );
#if debug
    printf( "in 2nd vertical bar loop, i = %d, thisspos = %d\n", i, thisspos );
#endif
    printSpaces( (thisspos - lastspos - 1 ) * 4 + 3 );
    putchar( '|' );
    lastspos = thisspos;
  }
 putchar('\n');
 for( i=0; i < slen; i++ ) /* write out the song string */
  {
    printf( "%4d", songgrid[i][i] );
  }
putchar('\n');
}

void writeSubArray( unsigned char *arr, int startpos, int endpos )
{
unsigned char *ptr = arr + startpos;
unsigned char *endptr = arr + endpos;
while( ptr != endptr )
  printf( "%4d", *ptr++ );
}

void showInterleavedMatchSpots( int qlen, int slen, int *query, 
				short **song,
				int *matchspots, int best )
{
  /* Note: arg best contains the number of matches stored in matchspots */
  int i;
  int diff, sdiff; /* diff between current elt and previous elt's line pos */
  int prevqpos, prevspos; /* position in line reached for previous elt */
  int thisqpos, thisspos; /* current line position for curr element*/
  int qi = 0, si = 0, bi = 0; /* current pos in string*/
  int qlinepos = 0, slinepos = 0, blinepos = 0; /* pos within current line */
  int qnote = 0, snote = 0;


while( si < best || qi < best || bi < best || qi < qlen || si < slen )
  {
    while( qlinepos < 20 )
      {
	/* show the query characters for this line*/
	/* first calculate position of next query character */

	/* then display it with the right number of spaces */
	/* then update all the appropriate counters and position holders */
      }
    while( blinepos < 20 )
      {
      /* show the match bars where appropriate for this line*/
	/* first calculate the position of the next match char */
	/* then display with the right number of spaces */
	/* then update all the appropriate counters */
      }
    while( slinepos < 20 )
      {
	/* show the next string characters for this line*/
	/* first calculate position of next song character */
	/* then display it with the right number of spaces */
	/* then update all the appropriate counters and position holders */
      }
      /*    update counters*/
    si++;qi++;bi++;
  }


}

int fastSpecLCS( unsigned char *querystring, unsigned char *songstring,
		 int qlen, int slen, short **querygrid, short **songgrid,
		 int maxInserts, int minExacts )
{
  /* This performs an interval "longest common subsequence" calculation.
     It only checks up to "maxInserts" places for a match and insists
     on "minExacts" exact matches on the diagonal.  At the moment,
     minExacts is ignored and assumed to be 1.
  */
int i, j, srow, scol;
int best = 0, score;

#if DEBUG
  printf("fastSpecLCS called\n" );
#endif
for( i=0; i < qlen; i++ )
  for( j=i; j >= i - maxInserts; j-- )
    {
      /* start a match from this point against each possible location in the 
       song.
      */
      for( srow=0; srow < slen; srow++ )
	for( scol= srow; scol >= srow - maxInserts; scol-- )
	  if( querygrid[i][j] == songgrid[srow][scol] )
	    {
	    score = startFastSpecLCSMatch( querystring, songstring, qlen, slen,
			       querygrid, songgrid, i, j, srow, scol,
			       maxInserts );
	    best = MAX( score, best );
	    }
    }
return best;

}

int startSpecLCSMatch( unsigned char *querystring, unsigned char *songstring, 
		       int qlen, int slen,
		       short **querygrid, short **songgrid, 
		       int qrow, int qcol, int srow, int scol,
		       int *matchspots )
{
  int score = 1;
  int i = max( qrow, qcol ) + 1, j = i, si = max( srow, scol) + 1;
  int sj = si;

#if debug
	printf( "startSpecLCSMatch called, qrow=%d, qcol=%d, srow=%d, scol=%d\n",
		qrow, qcol, srow, scol );
#endif
while( i < qlen && j < qlen && si < slen && sj < slen )
  {
    if( querygrid[i][j] == songgrid[si][sj] )
      {
	*( matchspots + 0 + score* 2 ) = MAX( i, j );
	*( matchspots + 1 + score* 2 ) = MAX( si, sj);
	score++;
	nextSpot( &i, &j, &si, &sj, qlen, slen, MATCH ); 
#if debug
	printf( "match, i=%d, j=%d, si=%d, sj=%d, qlen=%d, slen=%d, score=%d\n"
		, i, j, si, sj,	qlen, slen, score );
#endif
      }
    else
      {
      	nextSpot( &i, &j, &si, &sj, qlen, slen, MISMATCH ); 
#if debug
	printf( 
	       "no match, i=%d, j=%d, si=%d, sj=%d, qlen=%d, slen=%d, score=%d\n"
	       , i, j, si, sj, qlen, slen, score );
#endif
      }
  }
return score; 
}

int startFastSpecLCSMatch( unsigned char *querystring, unsigned char *songstring, 
		       int qlen, int slen,
		       short **querygrid, short **songgrid, 
		       int qrow, int qcol, int srow, int scol, int maxInserts )
{
  int score = 1;
  int i = max( qrow, qcol ) + 1, j = i, si = max( srow, scol) + 1;
  int sj = si;

#if debug
	printf( "startFastSpecLCSMatch called, qrow=%d, qcol=%d, srow=%d, scol=%d\n",
		qrow, qcol, srow, scol );
#endif
while( i < qlen && j < qlen && si < slen && sj < slen )
  {
    if( querygrid[i][j] == songgrid[si][sj] )
      {
	score++;
	nextFastSpot( &i, &j, &si, &sj, qlen, slen, maxInserts, MATCH ); 
#if debug
	printf( "match, i=%d, j=%d, si=%d, sj=%d, qlen=%d, slen=%d, score=%d\n"
		, i, j, si, sj,	qlen, slen, score );
#endif
      }
    else
      {
      	nextFastSpot( &i, &j, &si, &sj, qlen, slen, maxInserts, MISMATCH ); 
#if debug
	printf( 
	       "no match, i=%d, j=%d, si=%d, sj=%d, qlen=%d, slen=%d, score=%d\n"
	       , i, j, si, sj, qlen, slen, score );
#endif
      }
  }
return score; 
}

void nextSpot( int *i, int *j, int *si, int *sj, int qlen, int slen, 
	       int match )
{
if( match )
  {
    *i = max( *i, *j ) + 1;
    *j = *i;
    *si = max( *si, *sj ) + 1;
    *sj = *si;
  }
else
  {
    if( *sj < slen - 1)
      (*sj)++;
    else
      {
	(*j)++;
	*sj = *si;
      }
  }
}

void nextFastSpot( int *i, int *j, int *si, int *sj, int qlen, int slen, 
		   int maxInserts, int match )
{
  if( match )
    {
      *i = max( *i, *j ) + 1;
      *j = *i;
      *si = max( *si, *sj ) + 1;
      *sj = *si;
    }
  else /* mismatch */
    {
      if( *sj < slen-1 && *sj < *si - maxInserts) /* not end of text row yet*/
	(*sj)++;
      else /* no match on the text row, so go skip a pattern symbol */
	{
	  (*j)++;
	  *sj = *si;
	}
    }
}

long editdistance( char *str1, char *str2, int mismatch, int indel, 
		   FLAGS myflags )
{
long **match;
long len1 = strlen( str1 );
long len2 = strlen( str2 );
int i;

if( ! (match = (long **) malloc( sizeof( long ) * len1 * len2 ) ) )
     doError( "Failed to allocate space for editdistance match structure\n" );

for( i=0; i < len1; i++ )
  match[0][i] = match[0][i-1] + sim( ' ', str2[i], mismatch, indel, myflags );

}

long meldistance( char *str1, char *str2 )
{
}

long **create2DArray( long len1, long len2 )
{
long i;
long **temp;

if( ! (temp = (long **) malloc( sizeof( long *) * len1 ) ) )
    {
    printf( "len1 is %d and we've failed\n", len1 );
     doError( "Failed to allocate space for match structure 1\n" );
    }
for( i=0; i< len1; i++ )
  if( ! (temp[i] = (long *) malloc( sizeof( long ) * len2 ) ) )
    {
      printf( "len1 is %d, len2 is %d and we've failed\n", len1, len2 );
     doError( "Failed to allocate space for match structure\n" );
    }
return temp;
}

short **createShort2DArray( long len1, long len2 )
{
long i;
short **temp;

if( ! (temp = (short **) malloc( sizeof( short *) * len1 ) ) )
  {
    printf( "len1 is %d and we've failed\n", len1 );
     doError( "Failed to allocate space for match structure 1\n" );
  }
for( i=0; i< len1; i++ )
  if( ! (temp[i] = (short *) malloc( sizeof( short ) * len2 ) ) )
    {
      printf( "len1 is %d, len2 is %d and we've failed\n", len1, len2 );
     doError( "Failed to allocate space for match structure\n" );
    }
return temp;
}

void freeArray( long **arr, int maxlen )
{
int i, j;
for( i=0; i < maxlen; i++ )
  free( arr[i] );
free( arr );
}

void freeShortArray( short **arr, int maxlen )
{
int i, j;
for( i=0; i < maxlen; i++ )
  free( arr[i] );
free( arr );
}

void showMatrix( short **arr, int maxlen, int len1, int len2, 
	    char *str1, char *str2 )
{
int i;

printf("\t");
for( i=0; i < len2; i++ )
  printf( "%c\t", *str2++ );
printf("\n");
showShort2Darray( arr, len1+1, len2+1 );
}

void show2Darray( long **arr, int dim1, int dim2 )
{
int i, j;

for( i=0; i < dim1; i++ )
  {
    for( j=0; j < dim2; j++ )
      printf( "%ld\t", arr[i][j] );
    printf( "\n" );
  }

}

void showShort2Darray( short **arr, int dim1, int dim2 )
{
int i, j;

for( i=0; i < dim1; i++ )
  {
    for( j=0; j < dim2; j++ )
      printf( "%ld\t", arr[i][j] );
    printf( "\n" );
  }

}

int uCharCountngrams( unsigned char *querystring, unsigned char *songstring, 
		      int qlen, int slen )
{
return 1;
}

void initUCharngramarr( NGRAMTREE arr[], int arrsize, FLAGS myflags )
{
int i;
for( i=0; i< arrsize; i++ )
  {
    arr->exists = 0;
    arr->left = arr->right = NULL;
    if( myflags.measuretype == 'u' || myflags.measuretype == 'c' )
      (*arr).numOcc = 0;
    arr++;
  }

}


void resetUCharngramarr( NGRAMTREE arr[], int arrsize, 
			 unsigned char *querystring, int qlen, FLAGS myflags )
{
int index, i, scanlength = qlen - myflags.ngramsize;
NGRAMTREE *ptr;
NGRAMTREE *previous;
int left;

#if DEBUG
printf( "resetUCharngramarr called\n" );
#endif
for( i=0; i <= scanlength; i++ )
  {
    ptr = ngHashUChar( querystring++, arr, arrsize, &previous, &left, 
		       myflags.ngramsize );
    if( ptr && ptr->exists )
      ptr->numOcc = 0;
  }
}

int nghashUCharArr( unsigned char *querystring, int ngsize )
{
int i, index = 0, alphabetsize = alphasize('i');
for( i=0; i < ngsize; i++ )
  {
#if debug > 1
    printf( "i==%d, *querystring is %d, index is %d\n", 
	    i, *querystring, index );
#endif
      {
	index = index * alphabetsize + (*querystring++);
      }
  }
#if debug
printf( " index calculated is %d\n", index );
#endif
return index; 
}


NGRAMTREE *ngHashUChar(unsigned char *querystring, NGRAMTREE arr[], 
		       int arrsize,
		      NGRAMTREE **previous, int *left, int ngramsize )
{
  /* This function is different to the other hash functions in that it returns
     a pointer to where the ngram would be stored.
     If the value of the pointer is null, then this means that there is
     nothing stored at that location yet (i.e. the ngram doesn't yet exist
     in the structure ).

  */
int i;
unsigned short index = 0;
NGRAMTREE *ptr, *prev = NULL;
char *hash2str;
unsigned char *q = querystring; /*used for debugging only */
int cmplength = ngramsize - 2;
*left = 0;
index =  (short) *((short *)querystring);
#if debug
printf( "ngHashUChar called with querystring index value as %d\n", index );
#endif

/* the second half of the calculation involves using the second half of the
   ngram to locate the binary tree entry at this array element
*/
hash2str = querystring + 2;
#if debug
printf( "hash2str is " );
showUChars( hash2str, cmplength );
printf( "querystring is " );
showUChars( querystring, 4 );
#endif
ptr = arr + index;

#if debug
printf( "About to loop through ptrs\n" );
#endif
while( ptr && ptr->exists  &&
       memcmp( ptr->part2value, hash2str, cmplength ) )
  {
#if debug
printf( "Inside loop through ptrs, ptr = %d\n", ptr );
#endif
    prev = ptr;
    if( memcmp( hash2str, ptr->part2value, cmplength ) < 0 )
      {
	ptr = ptr->left;
	*left = 1;
      }
    else
      {
	ptr = ptr->right;
	*left = 0;
      }
  }
/* at this point we either haven't found it, in which case ptr is equal to
   NULL, or we have found it, or for some reason ptr->exists is zero for a 
   stored ngram - shouldn't happen.
   We will want the previous location, if there was a NULL, so that
   a new entry can be created.
*/
#if debug
printf( "Finished loop through ptrs\n" );
#endif

*previous = prev;
return ptr;
}
 

void initngramarr( NGRAMS arr[], int arrsize, FLAGS myflags )
{
int i;
for( i=0; i< arrsize; i++ )
  {
    (*arr).exists = 0;
    if( myflags.measuretype == 'u' || myflags.measuretype == 'c')
      (*arr).numOcc = 0;
    arr++;
  }

}

int (*gethashfn( char notetype ))( char *, int )
{
switch( notetype )
  {
  case 'c': 
    return (int (*)( char *, int ))nghashContour;
    break;
  case 'C': 
    return (int (*)( char *, int ))nghashContourRhythmCombo;
    break;
  case 'i':
    return (int (*)( char *, int ))nghashUCharArr;
  case 'd':
  default:
    return (int (*)( char *, int ))nghashMod;
    break;
  }
}

void resetngramarr( NGRAMS arr[], int arrsize, char *querystring, int qlen,
	      int (*hash)( char *, int ), FLAGS myflags )
{
int index, i, scanlength = qlen - myflags.ngramsize;

#if DEBUG
printf("In resetngramarr, resetting the ngram array, hash fn is %d\n",
       hash );
#endif
for( i=0; i <= scanlength; i++ )
  {
#if debug
    printf( "About to hash " );
    showUChars( querystring, myflags.ngramsize );
#endif
    index = hash( querystring++, myflags.ngramsize );
    arr[index].numOcc = 0;
  }
}

int nghashMod( char *querystring, int ngsize )
     /* this one only works for modulo int */
{
int i, index = 0, alphabetsize = alphasize('d');
for( i=0; i < ngsize; i++ )
  {
#if debug > 1
    printf( "i==%d, *querystring is %c which equates to %d, index is %d\n", 
	    i, *querystring, *querystring, index );
#endif
    /* we need to use a different value depending on the characters used */
    if( *querystring != '.' )
      index = index * alphabetsize + (*querystring++ - 97);
    else
      {
	index = index * alphabetsize + alphabetsize - 1;
	*querystring++;
      }
  }
#if debug
printf( " index calculated is %d\n", index );
#endif
return index; 
}

int nghashContour( char *querystring, int ngsize )
     /* this one only works for contour */
{
int i, index = 0, alphabetsize = alphasize( 'c' );
for( i=0; i < ngsize; i++ )
  {
#if debug
    printf( "i==%d, *querystring is %c which equates to %d, index is %d\n", 
	    i, *querystring, *querystring, index );
#endif
    if( *querystring == 'D' )
      index = index * alphabetsize;
    else if( *querystring == 'R' )
      index = index * alphabetsize + 1;
    else if( *querystring == 'U' )
      index = index * alphabetsize + 2;
    else
      index = index * alphabetsize + 3;
    querystring++;
  }
#if debug
printf( " index calculated is %d\n", index );
#endif
return index; 
}

int nghashContourRhythmCombo( char *querystring, int ngsize )
     /* 
	this one only works for contour + rhythm stored in one char
	Longer durations are stored using UDR (+0), shorter as udr (+32)and same
	ones as 5$2 (-32)
      */
{
  int i, index = 0, alphabetsize = alphasize( 'C' );
  for( i=0; i < ngsize; i++ )
    {
#if debug
      printf( "i==%d, *querystring is %c which equates to %d, index is %d\n", 
	      i, *querystring, *querystring, index );
#endif
      switch( *querystring % 32)
	{
	case 4:
	  index = index * alphabetsize;
	  break;
	case 18:
	  index = index * alphabetsize + 1;
	  break;
	case 21:
	  index = index * alphabetsize + 2;
	  break;
	case 30:
	  index = index * alphabetsize + 3;
	  break;
	}
      index += (*querystring / 32 - 1) * 4;
      querystring++;
    }
#if debug
  printf( " index calculated is %d\n", index );
#endif
  return index; 
}

void scanUCharquery( NGRAMTREE arr[], int arrsize, unsigned char *querystring,
		     int querylength, FLAGS myflags )
{
  /*
    This function scans through the query and builds the ngram tree based
    on the ngrams found.
    Warning: the n-gram tree contains pointers to the query string.
  */
int  i, scanlength = querylength - myflags.ngramsize;
NGRAMTREE *ptr, *newEntry, *previous;
int left;

#if DEBUG
printf( "scanUCharquery called, querylength = %d, scanlength is %d\n", 
	querylength, scanlength );
printf( "query is: " );
showUChars( querystring, querylength );
#endif

for( i=0; i <= scanlength; i++ )
  {
    ptr = ngHashUChar( querystring, arr, arrsize, &previous, &left, 
		       myflags.ngramsize );
#if debug
printf( "scanUCharquery: querystring is " );
showUChars( querystring, 4 );
#endif
    if( !ptr )
      {
#if debug
	printf( "No existing node, so create one\n" );
#endif
	/* null pointer, so add an entry */
	if( !(newEntry = (NGRAMTREE *) malloc( sizeof( NGRAMTREE ) ) ))
	  doError( "Failed to create new hash overflow entry" );
	newEntry->exists = 1;
	newEntry->left = NULL;
	newEntry->right = NULL;
	newEntry->part2value = querystring + 2;
#if debug
printf( " part2value is" );
showUChars( newEntry->part2value, myflags.ngramsize - 2 );
#endif
	if( previous )
	  if( left )
	    previous->left = newEntry;
	  else
	    previous->right = newEntry;
      }
    else
      {
#if debug
	printf( "ptr is %li\n", ptr );
#endif
	if( !ptr->exists )
	  {
	    ptr->part2value = querystring + 2;
#if debug
printf( "existing array element part2value is %d\n", newEntry->part2value );
#endif
	  }
	ptr->exists++;
      }
    querystring++;
  } /*end for */
}

double scanUChartext( NGRAMTREE arr[], int arrsize, unsigned char *songstring,
	      int songlength, FLAGS myflags, int queryLength )
{
int index, i, scanlength = songlength - myflags.ngramsize;
double score = 0;
NGRAMTREE *ptr;
NGRAMTREE *previous;
int left;

/* This returns the similarity score of the query and the current song 
 using an n-gram measure
*/

if( myflags.measuretype == 'u' )
    score = -queryLength + myflags.ngramsize - 1;
 if( myflags.debuglevel > 1 )
   printf( "scanUChartext called, songlength = %d\n", songlength );
for( i=0; i <= scanlength; i++ )
  {
    ptr = ngHashUChar( songstring++, arr, arrsize, &previous, &left,
		       myflags.ngramsize );
    if( ptr && ptr->exists )
      {
	switch( myflags.measuretype )
	  {
	  case 't':
	  case 'n':
	    score++;
	    break;
	  case 'u':
	    ptr->numOcc++;
	    /* Using negative Ukkonen score, subtract old component and
	       add new component.
	    */
	    if( ptr->numOcc > ptr->exists )
	      score--;
	    else
	      score++;
	    break;
	  case 'i':
	  case 'L':
	    score += ptr->idf;
	    break;
	  case 'c':
	    if( !( ptr->numOcc ) )
	      {
		score++;
		ptr->numOcc++;
	      }
	  }
      }
    else
      {
	if( myflags.measuretype == 'u' )
	  score--;
      }
  }
#if debug
printf( "score calculated as %lf\n", score ); 
#endif
return score;
}

void scanquery( NGRAMS arr[], int arrsize, char *querystring, int qlen,
	      int (*hash)( char *, int ), FLAGS myflags )
{
int index, i, scanlength = qlen - myflags.ngramsize;

#if DEBUG
printf( "in scanquery, hash fn is %d and scanlength is %d\n", hash, 
	scanlength );
#endif
for( i=0; i <= scanlength; i++ )
  {
    index = hash( querystring++, myflags.ngramsize );
    arr[ index ].exists ++;
  }
}

double scantext( NGRAMS arr[], int arrsize, char *songstring, double *ngramfreqs,
	      int (*hash)( char *, int ), FLAGS myflags, int queryLength,
	      int songLength )
{
  /* Calculate ngram similarity score */
int index, i, scanlength = songLength - myflags.ngramsize;
double score = 0.0;
NGRAMS *arrptr = arr;

if( myflags.measuretype == 'u' )
  score = -queryLength + myflags.ngramsize - 1;
for( i=0; i <= scanlength; i++ )
  {
#if debug
    printf( "About to hash " );
    showUChars( songstring, myflags.ngramsize );
#endif
    index = hash( songstring++, myflags.ngramsize );
#if debug
    printf( "index calculated as %d and arr[index].exists = %d\n",
	    index, arr[index].exists );
    if( myflags.measuretype == 'i' )
      printf( "ngramfreqs[ index ] = %f\n", ngramfreqs[ index ] ) ;
#endif
    if( arr[ index ].exists )
      {
	arrptr = &arr[ index ];
	switch( myflags.measuretype )
	  {
	  case 'u':
	    arrptr = &arr[ index ];
	    arrptr->numOcc++;
	    /* Using negative Ukkonen score, subtract old component and
	       add new component.
	    */
	    if( arrptr->numOcc > arrptr->exists )
	      score--;
	    else
	      score++;
	    break;
	  case 't':
	  case 'n':
	    score++;
	    break;
	  case 'c':
	    if(!( arrptr->numOcc ))
	      {
		score++;
		arrptr->numOcc++;
	      }
	    break;
	  case 'i':
	    score += ngramfreqs[ index ];
if( myflags.debuglevel > 1 )
  printf( "adding %f to score, which is now %f\n", ngramfreqs[ index ], score );
	    break;
	  }
      }
    else
      {
	if( myflags.measuretype == 'u' )
	  {
	    /* We want to consider those where song has the ngram and 
	       query doesn't
	    */
	    score--;
	  }
      }
  }
#if DEBUG
printf( "In scantext, score calculated as %.2f, measuretype is %c\n", score,
	myflags.measuretype ); 
printf( "scanlength is %d\n", scanlength );
#endif
return score;
}

double countngrams( NGRAMS *arr, int arrsize, char *songstring, double *ngramfreqs,
		char *querystring, int queryLength, int songLength,
		double *docfreqs,  FLAGS myflags )
{
double score;
char *sptr = songstring;
int (*fptr)( char *, int );

#if DEBUG
 printf( "countngrams called\n" );
#endif
fptr = gethashfn( myflags.notetype );
if( fptr )
  {
    score = scantext( arr, arrsize, songstring, ngramfreqs, fptr, myflags, 
		      queryLength, songLength );
#if DEBUG
    if( myflags.measuretype == 'u' )
      {
	printf( "ukkonen measure being used, initial score is %d\n", score );
	printf( "After scanning text the array:\n" );
	showNgramArray( arr, arrsize );
      }
#endif
  }
else
    doError( "invalid note type for ngram counting\n" );
if( myflags.debuglevel > 0 )
/*showNgramArray( arr, arrsize );*/
  printf( "ngram score calculated as %f\n", score ); 
return score;
}

void showNgramArray( NGRAMS *arr, int arrsize )
{
int i, j = 0;
printf( "Showing Ngram Array (array size %d)...\n", arrsize );
for( i=0; i < arrsize; i++ )
  {
  if( arr[i].exists )
    {
#if DEBUG
      printf( "j=%d ", j++ );
#endif
      printf( "arr[%d].exists = %d, arr[%d].numOcc = %d\n", i, 
	    arr[i].exists, i, arr[i].numOcc );
    }
  }
}

void showTree( NGRAMTREE *ptr, char dir, int ngramsize )
{
  if( ptr )
    {
      if( ptr->exists )
	{
	  printf( "%c exists = %d, idf = %lf, part2value = ", dir, 
		  ptr->exists, ptr->idf );
	  showUChars( ptr->part2value, ngramsize - 2 );
	  printf( "\n" );
	}
      showTree( ptr->left, 'L', ngramsize );
      showTree( ptr->right, 'R', ngramsize );
  }
 else
   printf( " end %c branch\n", dir );
}

void showNgramTree( NGRAMTREE *arr, int arrsize, int ngramsize )
{
int i;
NGRAMTREE *ptr;

printf( "Showing Ngram Tree (array size %d)...\n", arrsize );
for( i=0; i < arrsize; i++ )
  {
    if( arr[i].exists )
      {
	printf( "arr[%d].exists = %i, idf = %lf, part2value = \n", i, 
	    arr[i].exists, arr[i].idf );
	showUChars( arr[i].part2value, ngramsize - 2 );
	ptr = &(arr[i]);
	showTree( ptr->left, 'L', ngramsize );
	showTree( ptr->right, 'R', ngramsize );
      }
  }
}

int longestBranch( NGRAMTREE *ptr )
{
  if( ptr )
    {
      return 1 + max(longestBranch( ptr->left ),
		     longestBranch( ptr->right ));
    }
 else
   return 0;
}

int longestNgramTreeBranch( NGRAMTREE *arr, int arrsize )
{
int i;
NGRAMTREE *ptr;
int maxlength = 0;

printf( "Calculating Ngram Tree's longest branch (array size %d)...\n",
	arrsize );
for( i=0; i < arrsize; i++ )
  {
#if DEBUGr
    printf( "i=%d ", i );
#endif
    if( arr[i].exists )
      {
	ptr = &(arr[i]);
	maxlength = max( max( longestBranch( ptr->left ), 
			      longestBranch( ptr->right )), maxlength );
      }
  }
return maxlength;
}


void showFormattedTree( NGRAMTREE *ptr, char dir, int ngramsize, int index )
{
  int j;

  if( ptr )
    {
      if( ptr->part2value )
        {
          printf( "%c%c", (char) index, (char) *((char *)&index + 1) );
          for( j=0; j < ngramsize - 2; j++ )
            putchar( ptr->part2value[j] );
          printf( "\t\t%d\t%lf\n", ptr->exists, ptr->idf );
        }
      showFormattedTree( ptr->left, 'L', ngramsize, index );
      showFormattedTree( ptr->right, 'R', ngramsize, index );
  }
  /* else
     printf( " end %c branch\n", dir );*/
}

void showFormattedNgramTree( NGRAMTREE *arr, int arrsize, 
                             int ngramsize, FLAGS myargs )
{
int i, j;
NGRAMTREE *ptr;

/* This displays ngrams as strings, so should only be done for
   displayable ngrams 
*/
printf( "\nShowing Ngram Tree (array size %d)...\n", arrsize );
 printf( "n-gram\tindex\texists\tidfvalue\n" );
for( i=0; i < arrsize; i++ )
  {
    if( arr[i].part2value || myargs.debuglevel > 3 )
      {
        printf( "%c%c", (char) i, (char) *((char *)&i + 1) );
        if( arr[i].part2value )
          for( j=0; j < ngramsize - 2; j++ )
            putchar( arr[i].part2value[j] );
        printf( "\t%i\t%d\t%lf\n", i, arr[i].exists, arr[i].idf );
        ptr = &(arr[i]);
        showFormattedTree( ptr->left, 'L', ngramsize, i );
        showFormattedTree( ptr->right, 'R', ngramsize, i );
      }
  }
}

