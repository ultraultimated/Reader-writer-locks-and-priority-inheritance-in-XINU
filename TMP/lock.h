				
#define	NLOCKS		50		

#define	READ		0
#define	WRITE		1		
#define	LNONE		3
#define	LFREE		4
#define	LUSED		5		

struct	lentry	{			
	int	lhead;			
	int	ltail;
	int noofreaders;
	int allprocess[NPROC];
	int created;
	char	lstate;			
	char	ltype;
};

extern	struct	lentry	locadd[];
extern	int	nextloc;
extern  int 	globallock;


void linit();
int lcreate();
int ldelete(int);
int lock(int, int, int);
int releaseall(int, int, ...);
