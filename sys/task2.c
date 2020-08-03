#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <lock.h>

#define DEFAULT_LOCK_PRIO 20

#define assert(x,error) if(!(x)){ \
            kprintf(error);\
            return;\
            }
/*int mystrncmp(char* des,char* target,int n){
    int i;
    for (i=0;i<n;i++){
        if (target[i] == '.') continue;
        if (des[i] != target[i]) return 1;
    }
    return 0;
}*/


void lp1(int lck){
    kprintf("%s(priority = %d) is requesting to enter critical section\n", proctab[currpid].pname, getprio(currpid));    
    if(lock(lck, WRITE, DEFAULT_LOCK_PRIO) == OK){
        kprintf("%s(priority = %d) has entered critical section\n", proctab[currpid].pname, getprio(currpid));
        sleep(1);
        //sleep(1);
        int i = 0;
        for(i; i < 100;i++){
                kprintf("A");
            //    sleep(1);
        }
        kprintf("\n%s has completed critical section(ramped up priority = %d)\n", proctab[currpid].pname, getprio(currpid));
        releaseall(1, lck);
        kprintf("%s original priority = %d\n", proctab[currpid].pname, getprio(currpid));
    }
}

void lp2(int lck){
    kprintf("%s(priority = %d) has started\n", proctab[currpid].pname, getprio(currpid));
    sleep(1);
    int i = 0;
    for(i; i < 100;i++){
            kprintf("B");
            //sleep(1);
    }
    kprintf("\n%s has completed its execution\n");
}

void lp3(int lck, int pr1){
    kprintf("%s(priority = %d) is requesting to enter critical section\n", proctab[currpid].pname, getprio(currpid));
    kprintf("Hence, ramping up the priority of %s\n", proctab[pr1].pname, getprio(pr1));
    if(lock(lck, WRITE, DEFAULT_LOCK_PRIO) == OK){
            kprintf("%s(priority = %d) has entered critical section\n", proctab[currpid].pname, getprio(currpid));
            int i = 0;
            for(i; i < 100;i++){
                    kprintf("C");
              //      sleep(1);
            }
            kprintf("\n%s has completed critical section\n", proctab[currpid].pname);
            releaseall(1, lck);
    }
}

void test_locks(){
    int lck;
    lck = lcreate();

    int pr1 = create(lp1, 2000, 10, "p1", 1, lck);
    int pr2 = create(lp2, 2000, 20, "p2", 1, lck);
    int pr3 = create(lp3, 2000, 30, "p3", 2, lck, pr1);
    
    resume(pr1);
  //  sleep(1);
    resume(pr2);
    sleep(1);
    resume(pr3);
    sleep(5);

    ldelete(lck);
}

/* to test semaphore */
void p1(int sem){
        kprintf("%s(priority = %d) is requesting to enter critical section\n", proctab[currpid].pname, getprio(currpid));
        if(wait(sem) == OK){               
                kprintf("%s(priority = %d) has entered critical section\n", proctab[currpid].pname, getprio(currpid));
                sleep(1);
                //sleep(1);
                int i = 0;
                for(i; i < 100;i++){
                        kprintf("A");
                    //    sleep(1);
                }
                kprintf("\n%s has completed critical section(priority = %d)\n", proctab[currpid].pname, getprio(currpid));
                signal(sem);
        }
}

void p2(int sem){
        kprintf("%s(priority = %d) has started\n", proctab[currpid].pname, getprio(currpid));
        sleep(1);
        int i = 0;
        for(i; i < 100;i++){
                kprintf("B");
                //sleep(1);
        }
        kprintf("\n%s has completed its execution\n");
}

void p3(int sem){
    kprintf("%s(priority = %d) is requesting to enter critical section\n", proctab[currpid].pname, getprio(currpid));
    if(wait(sem) == OK){
            kprintf("%s(priority = %d) has entered critical section\n", proctab[currpid].pname, getprio(currpid));
            int i = 0;
            for(i; i < 100;i++){
                    kprintf("C");
              //      sleep(1);
            }
            kprintf("\n%s has completed critical section\n", proctab[currpid].pname);
            signal(sem);
    }
}

void test_semaphore(){
        int sem;
        sem = screate(1);

        int pr1 = create(p1, 2000, 10, "p1", 1, sem);
        int pr2 = create(p2, 2000, 20, "p2", 1, sem);
        int pr3 = create(p3, 2000, 30, "p3", 1, sem);

        resume(pr1);
        //sleep(1);
        resume(pr2);
        sleep(1);
        resume(pr3);
        sleep(5);



}

int task2( )
{
        kprintf("--------------- Testing with Locks ---------------------- \n");
        test_locks();
        sleep(5);
        kprintf("--------------- Testing with Semaphores ----------------- \n");
        test_semaphore();
        shutdown();
}


