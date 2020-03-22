/* includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "melstrings.h"

char *getRoot( char *filename, char *compresstype )
     /*
       getRoot - finds the root part of a melody filename
       any directory information is removed from the name
       if the compresstype is n, the name is truncated after the track no
       if the compresstype is c, truncation occurs
      */
{
  char *root, *endRoot, *temp;
  char *mainName, *endName;
  int namelen;

  mainName = strrchr( filename, '/' );
  if( !mainName ) /* no slashes in the name */
    mainName = filename;
  else
    mainName++; /* immediately after the last slash */
  temp = strdup ( mainName );
  endName = strrchr( temp, '.' );
  if( endName )
    *endName = '\0';
  switch( *compresstype )
    {
    case 'c':
    case 'p':
      endRoot = strrchr( temp, 't' );
      if( endRoot )
	*endRoot = '\0';
      break;
    default:
      break;
    }
  root = strdup( temp );
  free( temp );
  return root;
}

