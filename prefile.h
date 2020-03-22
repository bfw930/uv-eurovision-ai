/* constants */

#define NOTEBUFLENGTH 1000


/* Function prototypes */
char *putFileInMemory( char *filename, int *fsize, FLAGS myflags );
SONG *getSongFile( char *filename, int maxLength, FLAGS myflags );
void displayMelody( SONG *dataPtr );
char *getString( SONG *song, FLAGS myflags );
char *getModDirString( SONG *song, FLAGS myflags );
char *getContourString( SONG *song, FLAGS myflags );
void freeSong( SONG *song );
short *getRelArray( SONG *song, int *length, FLAGS myflags );
short *getAbsArray( SONG *song, FLAGS myflags );
unsigned char*getUCharArray( SONG *song, int *length, FLAGS myflags );
void showUChars( unsigned char *str, int length );
