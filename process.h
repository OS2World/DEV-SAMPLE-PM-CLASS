#ifndef PROCESS_H
#define PROCESS_H

#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS


#include "tree.h"


class process;
class threadTree;
class windowTree;


#define idForeachThreadLast idForeachChainElementLast


#define idGetChainElementThreadThis 0
#define idGetChainElementThreadPTIB 1
//#define idGetChainElementThreadPProcess 2
#define idGetChainElementThreadLast 1
class thread:public construct, public chainElement
{	public:
	PTIB pTIB;
	process *pProcess;		// Backpointer
	// The first thread which is already started
	// should not be created using DosCreateThread()
	// bCreate = FALSE should be used for the first thread
	thread(Boolean bCreate, process *pProcessNew);
	// Calls every time DosCreateThread
	// Can be used if the thread will not be bound to the process
	// thread chain, but to a window thread chain
	thread(process *pProcessNew, chain *pParent);
	// DosCreateThread will be called during processing
	// of thread constructor.
	// Then the thread waits until startThread() was been called.
	// This is done, to avoid calling thread.run() before the
	// constructor of the derived class returns.
	void startThread(void)
	{	DosPostEventSem(hEventConstructed);
	}
	void createThread(void);
	HEV hEventConstructed;
	virtual ~thread(void);
	virtual void *getChainElementMember(unsigned int iMsg)
	{	switch (iMsg)
		{	default:
				return chainElement::getChainElementMember(iMsg);
			case idGetChainElementThreadThis:
				return (void*)this;
			case idGetChainElementThreadPTIB:
				return (void*)pTIB;
			//case idGetChainElementThreadPProcess:
				//return (void*)pProcess;
		}
	}
};


#define idGetChainProcessThis 0
#define idGetChainProcessLast 1
class process:public construct, public chain
{	public:
	process(int argc, char **argv);
	// thread with tid == 1
	thread *pProcessThread;
	// process info block
	PPIB pPIB;
	char **argv;
	int argc;
	virtual ~process(void);
	virtual Boolean create(void);
	void doSomething(void);
	void destruct(void);
	virtual void *getChainMember(unsigned int iMsg)
	{	switch (iMsg)
		{	default:
				abort();
				return "Hallo Peter";
			case idGetChainProcessThis:
				return (void*)this;
		}
	}
};


#endif /* !PROCESS_H */
