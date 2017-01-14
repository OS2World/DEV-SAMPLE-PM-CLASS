#include "process.h"
#ifdef INCLUDESOURCE
#include "tree.cc"
#endif


typedef struct helpStruct
{	HEV hev;
	thread *pThread;
};


static void _syscall helpFct(struct helpStruct *pHelp)
{	PIB *pPIB;
	thread *pThread = pHelp->pThread;
	chain *pParent;

	DosGetInfoBlocks(&pThread->pTIB, &pPIB);
	DosPostEventSem(pHelp->hev);
	DosWaitEventSem(pThread->hEventConstructed, (unsigned int)SEM_INDEFINITE_WAIT);
	DosCloseEventSem(pThread->hEventConstructed);
	pThread->run();
	(pParent = pThread->pParent)->getMutualExclusiveAccess();
	delete pThread;
	pParent->freeMutualExclusiveAccess();

}


thread::thread(process *pProcessNew, chain *pParent)
    :chainElement(pParent)
{
	pProcess = pProcessNew;
	createThread();
}


void thread::createThread(void)
{	struct helpStruct help;
	TID tid;

	DosCreateEventSem((unsigned char*)0,
	    &hEventConstructed,
	    (unsigned long)0,
	    (BOOL32)0);
	help.pThread = this;
	DosCreateEventSem((unsigned char*)0,
	    &help.hev,
	    (unsigned long)0,
	    (BOOL32)0);
	DosCreateThread(&tid,
	   (void (_syscall *)(unsigned long))helpFct,
	   (unsigned long)&help,
	   (unsigned long)0,
	   (unsigned long)1024*1024);
	DosWaitEventSem(help.hev, (unsigned int)SEM_INDEFINITE_WAIT);
	DosCloseEventSem(help.hev);
}


thread::thread(Boolean bCreate, process *pProcessNew)
    :chainElement(pProcessNew)
{	PIB *pPIB;

	pProcess = pProcessNew;
	if (bCreate)
		createThread();
	else
	{	pProcess->pProcessThread = this;
		DosGetInfoBlocks(&pTIB, &pPIB);
	}
}


/*
I'm not so shure that this works,
but until now I did'nt get any crash.
The destructor of a thread can be called by more then one thread:
The thread with tid == 1 or the thread itself in helpFct().
To avoid that a thread object will be freed twice,
you should always call pParent->getMutualExclusiveAccess() and
check then wether the thread is not already freed before calling
the destructor.
*/
thread::~thread(void)
{	PIB *pPIBHelp;
	TIB *pTIBHelp;

	DosGetInfoBlocks(&pTIBHelp, &pPIBHelp);
	if (pTIBHelp->tib_ptib2->tib2_ultid != pTIB->tib_ptib2->tib2_ultid)
		DosKillThread(pTIB->tib_ptib2->tib2_ultid);
	if (pProcess->pProcessThread == this)
		pProcess->pProcessThread = (thread*)0;
}


process::process(int argcNew, char **argvNew):construct(), chain()
{	TIB *pTIB;

	DosGetInfoBlocks(&pTIB, &pPIB);
	argc = argcNew;
	argv = argvNew;
	pProcessThread = (thread*)0;
	pPIB = (PPIB)0;
}


Boolean process::create(void)
{	if (!new thread(FALSE, this))
		return FALSE;
	else
		if (pProcessThread->isSuccessfull())
			return TRUE;
		else
		{	delete pProcessThread;
			return FALSE;
		}
}


void process::destruct(void)
{       if (pProcessThread)
		delete pProcessThread;
}


void process::doSomething(void)
{	pProcessThread->run();
}


process::~process(void)
{
}
