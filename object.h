#ifndef OBJECT_H
#define OBJECT_H
#ifndef DEBUG
#define NDEBUG
#endif /* !DEBUG */
extern "C"
{
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#ifdef __EMX__
#include <alloca.h>
#endif


#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#include <os2.h>
};


typedef unsigned int Boolean;


#ifdef DEBUG
#define printFileLine() fprintf(stderr, "%s:%u\n", __FILE__, __LINE__)
#else
#define printFileLine() (void)1
#endif


/*
This is the basic class, all other classes should be derived from this one
*/
class object
{       // Should be set to FALSE by using setNoSuccess()
	// if something goes wrong in a constructor
	Boolean bSuccessfullConstructed;
	public:
	inline Boolean isSuccessfull(void)
	{	return bSuccessfullConstructed;
	};
	inline void setNoSuccess(void)
	{	bSuccessfullConstructed = FALSE;
	};
	object(void);
	virtual ~object(void);
	void *operator new(unsigned int iSize);
	void operator delete(void *pObject);
};


/*
This class should be used to avoid access of more then one thread to a class.
The thread which owns the lock can call getMutualExclusiveAccess()
more then once.
I DO NOT KNOW WETHER THIS IS DONE WITHOUT BUGS!
*/
class mtxObject:virtual public object
{       HMTX hMtxAccess;
	public:
	mtxObject(void):object()
	{       if (DosCreateMutexSem((unsigned char*)0, &hMtxAccess, 0, FALSE))
			setNoSuccess();
	}
	virtual ~mtxObject(void)
	{       DosEnterCritSec();
		if (hMtxAccess)
			DosCloseMutexSem(hMtxAccess);
		DosExitCritSec();
	}
	virtual void getMutualExclusiveAccess(void)
	{       DosRequestMutexSem(hMtxAccess,
		    (unsigned int)SEM_INDEFINITE_WAIT);
	}
	virtual void freeMutualExclusiveAccess(void)
	{       DosReleaseMutexSem(hMtxAccess);
	}
};


/*
This class should be used for windows, threads and processes.
See object.cpp and the implementation of process/threads/windows.
If something goes wrong in the init function FALSE should be returned.
More in object.cpp...
*/
class construct:virtual public object
{	public:
	construct(void):object()
	{
	}
	virtual void run(void);
	virtual void doSomething(void);
	virtual Boolean init(void);
	virtual Boolean initPre(void);
	virtual Boolean create(void);
	virtual Boolean initPost(void);
	virtual void destructPost(void);
	virtual void destructPre(void);
	virtual void destruct(void);
	virtual void destructAll(void);
};


#endif /* !OBJECT_H */
