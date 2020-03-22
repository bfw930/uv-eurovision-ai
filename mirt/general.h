/* constants */
#define BUFLENGTH 100


/* macros */

#define MAX( i, j )  ((i)>(j) ? (i): (j) )

/*	function prototypes	*/
void doError( char *message, ... );
void stripCR( char *string );
void strToLower( char *string );
int wordSearch( char *arr[], int numElements, char *theWord );
char *getLargeString();
char *fgetLargeString( FILE *infile );
int max( int i, int j );
int min( int i, int j );
char **allocArrayElement( int *storeLength, void *arrayptr, int reallocSize,
			int ptrSize, int structureSize, int pos );

