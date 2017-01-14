#ifndef PMPROCESS_H
#define PMPROCESS_H


#define INCL_WIN


#include "process.h"


class pmMsgThread;


#define idForeachWindowTreeCallInit (idForeachTreeLast + 1)
#define idForeachWindowTreeWinDestroyWindow (idForeachTreeLast + 2)
#define idForeachWindowTreeCallDestructAll (idForeachTreeLast + 3)
#define idForeachWindowTreeCanClose (idForeachTreeLast + 4)
#define idForeachWindowTreeLast (idForeachTreeLast + 4)

#define idGetChainElementWindowTreeThis (idGetChainElementTreeLast + 1)
#define idGetChainElementWindowTreePMsgThread (idGetChainElementTreeLast + 2)
#define idGetChainElementWindowTreeHwndFrame (idGetChainElementTreeLast + 3)
#define idGetChainElementWindowTreeHwndClient (idGetChainElementTreeLast + 4)
#define idGetChainElementWindowTreeHWindowDC (idGetChainElementTreeLast + 5)
#define idGetChainElementWindowTreeClassStyle (idGetChainElementTreeLast + 6)
#define idGetChainElementWindowTreeWindowStyle (idGetChainElementTreeLast + 7)
#define idGetChainElementWindowTreePTitelBarText (idGetChainElementTreeLast + 8)
#define idGetChainElementWindowTreeFrameFlags (idGetChainElementTreeLast + 9)
#define idGetChainElementWindowTreeResourceId (idGetChainElementTreeLast + 10)
#define idGetChainElementWindowTreePWindowClassName (idGetChainElementTreeLast + 11)
#define idGetChainElementWindowTreeLast (idGetChainElementTreeLast + 11)


class windowTree:public tree, public construct
{	public:
	// pointer to the message thread.
	// actual this should be the thread with tid == 1
	pmMsgThread *pMsgThread;
	// contains threads, which are not assigned to the process.
	// This threads are doing only jobs for this window
	chain chainThreads;
	// The window has the focus
	Boolean bFocus;
	// Pointer to my birthday
	HWND hwndFrame;
	HWND hwndClient;
	HDC hWindowDC;
	// The following stuff can be written to in constructors
	// of derived classes
	ULONG classStyle;	ULONG windowStyle;
	char *pTitelBarText;
	ULONG frameFlags;
	ULONG resourceId;
	char *pWindowClassName;
	/*windowProperties *pWindowProperties;*/
	windowTree *getParentWindow(void)
	{	return getDepth() > 1
		    ? (windowTree*)pParent->getChainMember(
		    idGetChainElementWindowTreeThis)
		    : (windowTree*)0;
	}
	windowTree(pmMsgThread *pMsgThreadNew, int idResource, char *pTitle);
	windowTree(windowTree *pParent, int idResource, char *pTitle);
	virtual MRESULT windowProc(ULONG msg, MPARAM mp1, MPARAM mp2);
	virtual MRESULT WMPaint(MPARAM mp1, MPARAM mp2);
	virtual void doPaint(HPS hps, RECTL *pStructRectl);
	virtual ~windowTree(void);
	//void runWindow(void);
	virtual Boolean create(void);
	virtual MRESULT WMClose(MPARAM mp1, MPARAM mp2);
	virtual MRESULT WMDestroy(MPARAM mp1, MPARAM mp2);
	virtual MRESULT WMFocusChange(MPARAM mp1, MPARAM mp2);
	virtual Boolean canClose(void);
	virtual Boolean canCloseHierarchy(void)
	{	if (foreach(idForeachWindowTreeCanClose, (void*)0))
			return FALSE;
		else
			return TRUE;
	}	virtual int toBeCalledPost(unsigned int iMsg, void *pDummy);	virtual int toBeCalledPre(unsigned int iMsg, void *pDummy);
	virtual Boolean initPost(void);
	virtual void *getChainElementMember(unsigned int iMsg)
	{	switch (iMsg)
		{	default:
				return tree::getChainElementMember(iMsg);
/*
pmMsgThread *pMsgThread;	 // Backpointer
	//windowTree *pParentWindow;
		HWND hwndFrame;
	HWND hwndClient;
	HDC hWindowDC;
	ULONG classStyle;
	ULONG windowStyle;
	char *pTitelBarText;
	ULONG frameFlags;
	ULONG resourceId;
	char *pWindowClassName;
*/
			case idGetChainElementWindowTreeThis:
				return (void*)this;
			case idGetChainElementWindowTreePMsgThread:
				return (void*)pMsgThread;
			case idGetChainElementWindowTreeHwndFrame:
				return (void*)hwndFrame;
			case idGetChainElementWindowTreeHwndClient:
				return (void*)hwndClient;
			case idGetChainElementWindowTreeHWindowDC:
				return (void*)hWindowDC;
			case idGetChainElementWindowTreeClassStyle:
				return (void*)classStyle;
			case idGetChainElementWindowTreeWindowStyle:
				return (void*)windowStyle;
			case idGetChainElementWindowTreePTitelBarText:
				return (void*)pTitelBarText;
			case idGetChainElementWindowTreeFrameFlags:
				return (void*)frameFlags;
			case idGetChainElementWindowTreeResourceId:
				return (void*)resourceId;
			case idGetChainElementWindowTreePWindowClassName:
				return (void*)pWindowClassName;
		}
	}
};


#define idGetChainElementProcessWindow idGetChainElementWindowTreeLast
class processWindow:public windowTree
{	public:
	processWindow(pmMsgThread *pMsgThreadNew, int idResource, char *pTitle)
	    :windowTree(pMsgThreadNew, idResource, pTitle)
	{}
	processWindow(windowTree *pParent, int idResource, char *pTitle)
	    :windowTree(pParent, idResource, pTitle)
	{}
	virtual MRESULT WMClose(MPARAM mp1, MPARAM mp2);
};


class pmProcess;


#define idForeachPmThreadLast idForeachThreadLast


#define idGetChainElementPmThreadThis (idGetChainElementThreadLast + 1)
#define idGetChainElementPmThreadHab (idGetChainElementThreadLast + 2)
#define idGetChainElementPmThreadLast (idGetChainElementThreadLast + 2)
class pmThread: public thread
{	public:
	HAB hab;
	pmThread(pmProcess *pProcessNew);
	pmThread(pmProcess *pProcessNew, chain *pParent);
	virtual ~pmThread(void);
	virtual void *getChainElementMember(unsigned int iMsg)
	{	switch (iMsg)
		{	default:
				return thread::getChainElementMember(iMsg);
			case idGetChainElementPmThreadThis:
				return (void*)this;
			case idGetChainElementPmThreadHab:
				return (void*)hab;
		}
	}
};


#define idForeachPmWindowThreadSetBackgroundPriority (idForeachPmThreadLast + 1)
#define idForeachPmWindowThreadSetForegroundPriority (idForeachPmThreadLast + 2)
#define idForeachPmWindowThreadLast (idForeachPmThreadLast + 2)
class pmWindowThread:public pmThread
{	public:
	windowTree *pWindow;
	//	Values for priority,
	//	dependent on the window owns the focus or not.
	//	Initialy the foreground values will be set to normal values,
	//	and the background values to idle state
	unsigned int iClassF, iValueF, iClassB, iValueB;
	pmWindowThread(windowTree *pWindowNew);
	virtual int toBeCalledForeachElement(unsigned int iMsg, void *pDummy);
	virtual Boolean initPre(void);
};


#define idGetChainElementPmMsgThreadTreeThis (idGetChainElementPmThreadLast + 1)
#define idGetChainElementPmMsgThreadTreeHmq (idGetChainElementPmThreadLast + 2)
#define idGetChainElementPmMsgThreadTreeLast (idGetChainElementPmThreadLast + 2)
class pmMsgThread:public pmThread
{	public:
	HMQ hmq;
	// contains the windows assigned to this message thread
	tree structTreeWindows;
	//	windowTree *pWindowTree;
	pmMsgThread(pmProcess *pProcess);
	//	pmMsgThread(threadTree *pParentNew);
	virtual ~pmMsgThread(void);
	virtual Boolean create(void);
	virtual Boolean initPost(void);
	virtual void destruct(void);
	virtual void destructPre(void);
	virtual void doSomething(void);
	virtual void *getChainElementMember(unsigned int iMsg)
	{	switch (iMsg)
		{	default:
				return pmThread::getChainElementMember(iMsg);
			case idGetChainElementPmMsgThreadTreeHmq:
				return (void*)hmq;
			case idGetChainElementPmMsgThreadTreeThis:
				return (void*)this;
		}
	}
};


#define idGetChainPmProcessThis (idGetChainProcessLast + 1)
#define idGetChainPmProcessLast (idGetChainProcessLast + 1)
class pmProcess: public process
{	public:
	//	virtual void runProcess(void);
	virtual Boolean create(void);
	pmProcess(int argc, char **argv):process(argc, argv)
	{
	}
	virtual void *getChainMember(unsigned int iMsg)
	{	switch (iMsg)
		{	default:
				return process::getChainMember(iMsg);
			case idGetChainPmProcessThis:
				return (void*)this;
		}
	}};


#endif PMPROCESS_H
