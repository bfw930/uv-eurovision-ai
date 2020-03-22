/* meldb.h */

/* Constants */


/* Function prototypes */
FILE *getDBPtr( char *filename );
void sortResults( RESULTLIST *msrsPtr );
void displayResults( RESULTLIST *msrsPtr, int numToDisplay, FLAGS myflags   );
RESULTLIST *initResultList();
double processMelody( SONG *song, SONG *query,
		   char *querystring, int queryLength,
		   char *songstring, int songLength,
		    int mismatch, int indel,
		   void *measureStructurePtr, int arrsize, FLAGS myflags );
char *getRoot( char *filename, char *compresstype );
int scoreCmp( RESULT **res1, RESULT **res2 );
void displayRecallFormat( RESULTLIST *results, int numReturned, 
			  char *queryname, FLAGS myflags );
void displayIncipitFormat( RESULTLIST *results, int numReturned, 
			  char *queryname, FLAGS myflags );
void displayKaraokeFormat( RESULTLIST *results, int numReturned, 
			  char *queryname, FLAGS myflags );
void displayHummingFormat( RESULTLIST *results, int numReturned, 
			  char *queryname, FLAGS myflags );
double getMeasure(char *querystring, int queryLength, char *songstring, 
       int songLength, int mismatch, int indel, void *measureStructurePtr, 
       int arrsize, double *docfreqs, FLAGS myflags, MATCHINFO *tempmatch );
char *keystring( char *str );
char *transpose( char *querystring, int diff );
long getShortMeasure(short *querystring, short *songstring, int qlen,
		     int slen, char *measuretype,
	       int mismatch, int indel );
void setFlagStruct( FLAGS *thestruct, int rest, int rhythm, int stress,
		    int absolute );
void getCompressedTextMeasures( char *query, FILE *dbPtr, 
				RESULTLIST *resultlist,
		     int mismatch, int indel, char *compresstype,
		     FLAGS myflags  );
NGRAMS *setupNgramCount( char *querystring, int qlen, int *asize, FLAGS myflags  );
double getUCharMeasure(unsigned char *querystring, unsigned char *songstring, 
	    int qlen, int slen, 
	    int mismatch, int indel, void *measureStructurePtr, int arrsize,
	    double *docfreqs, FLAGS myflags );
void displayFlags( FLAGS myflags );
NGRAMTREE *setupNgramTree( unsigned char *querystring, int *asize, 
			   int querylength, FLAGS myflags );
void initFlags( FLAGS *thestruct );
short **setupIntervalGrid( unsigned char *querystring,
			   int querylength, FLAGS myflags );
void freeIntervalGrid( short **grid, int querylength );
void showIntervalGrid( short **arr, int length );
int useNGramArr( FLAGS myflags );
double *initngramfreqs( int *numElts, FLAGS myflags  );
double *initDocFreqs( FLAGS myflags, int *numElts );
double *loadDocFreqs( char *filename, int *size, FLAGS myflags );
char *createIDFFileName( FLAGS myflags, char filetype  ) ;
void loadTreeDocFreqs( char *filename, NGRAMTREE *ngramtree, int arrsize, 
		       FLAGS myflags );
void addToTree( char *ngram, double idfvalue, NGRAMTREE *ngramtree,
		int arrsize, FLAGS myflags );
