#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>
#include <proc.h>
#include <sem.h>
#include <sleep.h>
#include <mem.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <stdio.h>
#include <lock.h>

int	nextloc;
int globallock;
struct	lentry	locadd[NLOCKS];

void linit(){
    int i = 0;
	globallock = -1;
    nextloc = NLOCKS-1;
	struct	lentry	*lptr;
	while(i < NLOCKS){
		int j;
		(lptr = &locadd[i])->lstate = LFREE;
		lptr->ltype = LNONE;
		lptr->noofreaders = 0;
		lptr->lhead = newqueue();
		lptr->ltail = 1 + (lptr->lhead);
		j = 0;
		do{
			lptr->allprocess[j] = LNONE;
			j++;
		}while(j < NPROC);
		i++;
	}
}
