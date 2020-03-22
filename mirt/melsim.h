#define MAX_MEL 100
#define BLANKINT 100000

typedef struct ngramrec
{
  short exists;	/* how many exist in the query pattern */
  short numOcc; /* how many exist in the text */
} NGRAMS;

typedef struct ngramtree
{
  char *part2value; /* all but first two chars of ngram */
  short exists; /* how many exist in the query */
  short numOcc; /* how many exist in the song */
  double idf; /* log  (numdocs/docfreq)  */
  struct ngramtree *left;
  struct ngramtree *right;
}NGRAMTREE;



/* macros for greater efficiency... */

#define SIM( x, y, mismatch, indel, match ) ((x)==(y)?(match):((x)==' '?(indel):(mismatch)))

/* Function prototypes 	*/
int alphasize( char notetype );
long hamming( char *str1, char *str2 );
long minhamming( char *str1, char *str2 );
long editdistance( char *str1, char *str2, int mismatch, int indel, FLAGS myflags  );
long meldistance( char *str1, char *str2 );
long localalignment( char *str1, char *str2, int mismatch, int indel, FLAGS myflags );
int sim( char c1, char c2, int mismatch, int indel, FLAGS myflags  );
int simInt( int c1, int c2, int mismatch, int indel, FLAGS myflags );
void show2Darray( long **arr, int dim1, int dim2 );
long **create2DArray( long len1, long len2 );
short **createShort2DArray( long len1, long len2 );
void showMatrix( short **arr, int maxlen, int len1, int len2, 
	    char *str1, char *str2 );
void freeArray( long **arr, int maxlen );
void freeShortArray( short **arr, int maxlen );
long smalllocalalignment( char *str1, char *str2, int mismatch, int indel, FLAGS myflags );
long smallLCS( char *str1, char *str2, FLAGS myflags );
long shortLCS( short *str1, short *str2, int len1, int len2 );
long shortlocalalignment( short *str1, short *str2, 
			  int len1, int len2, int mismatch, int indel, FLAGS myflags );
void showArray( int *array, int max, char *name );
long specialLocalalignment( void *str1, void *str2, 
			    int len1, int len2, 
			    int (*ssim)(void *, void *, int ),
			    int elementSize,
			   int mismatch, int indel, FLAGS myflags );
int simNotes( NOTE **n1, NOTE **n2, int mismatch, FLAGS myflags );


int shortCountngrams( short *querystring, short *songstring, 
		      int qlen, int slen );
double countngrams( NGRAMS *arr, int arrsize, char *songstring, double *ngramfreqs,
		 char *querystring, int queryLength, int songLength,
		 double *docfreqs, FLAGS myflags  );
int nghashUCharArr( unsigned char *querystring, int ngsize );
int nghashMod( char *querystring, int ngsize );
int nghashContour( char *querystring, int ngsize );
void scanquery( NGRAMS arr[], int arrsize, char *querystring, int qlen,
	      int (*hash)( char *, int ), FLAGS myflags );
double scantext( NGRAMS arr[], int arrsize, char *songstring, double *ngramfreqs,
	      int (*hash)( char *, int ), FLAGS myflags, int queryLength,
	      int songLength );
void showNgramArray( NGRAMS arr[], int arrsize );
void resetngramarr( NGRAMS arr[], int arrsize, char *querystring, int qlen,
	      int (*hash)( char *, int ), FLAGS myflags );
void initngramarr( NGRAMS arr[], int arrsize, FLAGS myflags );
void scanUCharquery( NGRAMTREE arr[], int arrsize, unsigned char *querystring,
		     int querylength, FLAGS myflags );
NGRAMTREE *ngHashUChar(unsigned char *querystring, NGRAMTREE arr[], 
		       int arrsize,
		      NGRAMTREE **previous, int *left, int ngramsize );
void showTree( NGRAMTREE *ptr, char dir, int ngramsize );
void showNgramTree( NGRAMTREE *arr, int arrsize, int ngramsize );
int longestNgramTreeBranch( NGRAMTREE *arr, int arrsize );
int longestBranch( NGRAMTREE *ptr );
void resetUCharngramarr( NGRAMTREE arr[], int arrsize, 
			 unsigned char *querystring, int qlen, FLAGS myflags );
int (*gethashfn( char notetype ))( char *, int );
double scanUChartext( NGRAMTREE arr[], int arrsize, unsigned char *songstring,
	      int songlength, FLAGS myflags, int queryLength );
void showShort2Darray( short **arr, int dim1, int dim2 );
int specLCS( unsigned char *querystring, unsigned char *songstring, 
	     int qlen, int slen, short **querygrid, short **songgrid );
int startSpecLCSMatch( unsigned char *querystring, unsigned char *songstring, 
		       int qlen, int slen,
		       short **querygrid, short **songgrid, 
		       int qrow, int qcol, int srow, int scol,
		       int *matchspots );
void nextSpot( int *i, int *j, int *si, int *sj, int qlen, int slen, 
	       int match );
int fastSpecLCS( unsigned char *querystring, unsigned char *songstring, 
	     int qlen, int slen, short **querygrid, short **songgrid,
		 int maxInserts, int minExacts );
int startFastSpecLCSMatch( unsigned char *querystring, unsigned char *songstring, 
		       int qlen, int slen,
		       short **querygrid, short **songgrid, 
		       int qrow, int qcol, int srow, int scol, int maxInserts );
void nextFastSpot( int *i, int *j, int *si, int *sj, int qlen, int slen, 
	       int maxInserts, int match );
void showMatchSpots( int qlen, int slen, short **querygrid, short **songgrid,
	     int *matchspots, int best );
int nghashContourRhythmCombo( char *querystring, int ngsize );
long specLocalAlignment( char *str1, char *str2, int mismatch, int indel,
			 FLAGS myflags );
long specUCharLocalAlignment( unsigned char *str1, unsigned char *str2, 
			  int len1, int len2, int mismatch, int indel, FLAGS myflags );
long uCharlocalalignment( unsigned char *str1, unsigned char *str2, 
			  int len1, int len2, int mismatch, int indel,
			  FLAGS myflags );

long globalqueryalignment( char *str1, char *str2, int mismatch, int indel,
			   FLAGS myflags );
long mixedalignment( char *str1, char *str2, int mismatch, int indel,
		     FLAGS myflags );
long mixedtbalignment( char *str1, char *str2, int mismatch, int indel,
		     FLAGS myflags, MATCHINFO *matchinfo );
