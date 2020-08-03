/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <lock.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
 
 int pidd;
 void proc(){
     struct pentry *ptr;
     ptr = &proctab[pidd];
     if (ptr->pstate = PRWAIT) {
         int max = -9999, count = 0;
         int prev;
         for(prev = q[locadd[ptr->waitlock].ltail].qprev; prev != locadd[ptr->waitlock].lhead; prev = q[prev].qprev ){
             if (max < proctab[prev].pprio) {
                 max = proctab[prev].pprio;
             }
         }
         int i;
         struct lentry *lptr;
         lptr = &locadd[ptr->waitlock];
         while(i < NPROC) {
             int check = lptr->allprocess[i] == READ || lptr->allprocess[i] == WRITE;
             if (check) {
                 if (max > proctab[i].pinh) {
                     chprio(i, max);

                 } else {
                     chprio(i, proctab[i].pinh);
                 }
             }
             i++;
         }
     }
}
 
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	pidd = pid;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:
	dequeue(pid);
	proc();
	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
