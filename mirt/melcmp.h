#define MAX_MEL 100

/*#include "melodies.h"*/
#include "general.h"

typedef struct simInfoRec
{
  char *str1offset; /* offset within str1 where longest alignment occurs */
  char *str2offset; /* offset within str2 where longest alignment occurs */
  long measure;
} SIMINFO;



/* Function prototypes 	*/
SIMINFO *melcmp( char *str1, char *str2 );
void showSimInfo( SIMINFO *info );

