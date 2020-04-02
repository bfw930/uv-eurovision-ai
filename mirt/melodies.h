/* melodies.h */

#define SIZE 20 
#define IDFFILE "/vin1/idf/idf"

typedef struct noteRec
{
short interval;
short rhythm;
short stress;
} NOTE;

typedef struct songRec
{
int songLength;
int storageLength;
char *fileName;
NOTE **notes;
} SONG;

typedef struct melodyListRec
{
long numSongs;	/*	number of songs stored in the structure	*/
long spaceAllocated; /*	number of song pointer spaces allocated	*/
SONG **list; /*	the array of pointers to songs	*/
} MELODYLIST;

typedef struct matchInfoRec
{
int qbestpos; /*	position in query that best score occurred     */
int sbestpos; /*	position in song that best score occurred	*/
} MATCHINFO;

typedef struct resultRec
{
  float score;	/* score calculated when compared to the query */
  char *songName;
 MATCHINFO *matchinfo;
} RESULT;

typedef struct resultListRec
{
long numSongs;	/*	number of songs stored in the structure	*/
long storeLength; /*	number of result pointer spaces allocated     */
RESULT **result; /*	the array of pointers to results	*/
} RESULTLIST;

typedef struct intSongRec /* this is used for preprocessed interval series */
{
int songLength;
int storageLength;
char *fileName;
short *notes;
} INTSONG;

typedef struct intSongListRec
{
long numSongs;	/*	number of songs stored in the structure	*/
long storeLength; /*	number of result pointer spaces allocated     */
INTSONG **intSong; /*	the array of pointers to results	*/
} INTSONGLIST;

typedef struct flags
{
short rests;
short rhythm;
short stress;
short normlog;
short normroot;
short verbose;
short fast;
short traceback;
char measuretype;
int ngramsize;
char notetype;
 char outputtype;
int windowsize;
int debuglevel;
int maxskips;
int match;
int smallmatch;
int mismatch;
int indel;
int noindels;
int nomismatch;
  char weighttype;
  int queryskips;
  int querystring;
  char *database;
} FLAGS;


/*	function prototypes	*/
float strRatio( char *low, char *high, char *word );
int melodySearch( char *arr[], int numElements, char *theWord );
int loadMelodyList( MELODYLIST *, char *filename);
int isInMelodylist( MELODYLIST *, char * );
