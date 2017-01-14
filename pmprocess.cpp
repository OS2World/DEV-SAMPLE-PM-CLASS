#include "PMProcess.h"
#ifdef INCLUDESOURCE
#include "process.cc"
#endif


static _syscall MRESULT pMProcessWindowProc(HWND hwnd,
    ULONG msg,
    MPARAM mp1,
    MPARAM mp2)
{	windowTree *pWindow;
	MRESULT ret;

	if (!(pWindow = (windowTree*)WinQueryWindowPtr(hwnd, 0)))
		return WinDefWindowProc(hwnd, msg, mp1, mp2);
	if (ret = pWindow->windowProc(msg, mp1, mp2))
		return ret;
	return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


windowTree::windowTree(pmMsgThread *pMsgThreadNew, int idResource, char *pTitle)
    :tree(&pMsgThreadNew->structTreeWindows), chainThreads()
{	frameFlags = (FCF_STANDARD) & ~(FCF_ICON | FCF_ACCELTABLE | FCF_MENU);
	windowStyle = WS_VISIBLE;
	classStyle = CS_SIZEREDRAW;
	/*| CS_CLIPSIBLINGS | CS_PARENTCLIP | CS_CLIPCHILDREN */
	pWindowClassName = "PMProcessClassName";
	pMsgThread = pMsgThreadNew;
	pTitelBarText = pTitle;
	resourceId = idResource;
	//pParentWindow = (windowTree*)0;
	//pMsgThread->pWindowTree = this;
}


windowTree::windowTree(windowTree *pParent, int idResource, char *pTitle)
    :tree(pParent), chainThreads()
{	frameFlags = (FCF_STANDARD) & ~(FCF_ICON | FCF_ACCELTABLE | FCF_MENU);
	windowStyle = WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	classStyle = CS_SIZEREDRAW | CS_CLIPSIBLINGS | CS_PARENTCLIP | CS_CLIPCHILDREN;
	pWindowClassName = "PMProcessClassName";
	pMsgThread = pParent->pMsgThread;
	pTitelBarText = pTitle;
	resourceId = idResource;
	//pParentWindow = pParent;
}


MRESULT windowTree::windowProc(ULONG msg, MPARAM mp1, MPARAM mp2)
{	switch (msg)
	{	case WM_CLOSE:
			return WMClose(mp1, mp2);
		case WM_PAINT:
			return WMPaint(mp1, mp2);
		case WM_DESTROY:
			return WMDestroy(mp1, mp2);
		case WM_FOCUSCHANGE:
			return WMFocusChange(mp1, mp2);
		default:
			return (MRESULT)FALSE;
	}
}


MRESULT windowTree::WMDestroy(MPARAM mp1, MPARAM mp2)
{	(void)mp1;
	(void)mp2;
	delete this;
	return (MRESULT)FALSE;
}


MRESULT windowTree::WMClose(MPARAM mp1, MPARAM mp2)
{	(void)mp1;
	(void)mp2;
	if (canCloseHierarchy())
	{	WinDestroyWindow(hwndFrame);
		return (MRESULT)TRUE;
	}
	else
		return (MRESULT)TRUE;
}


MRESULT windowTree::WMFocusChange(MPARAM mp1, MPARAM mp2)
{       (void)mp1;
	if (bFocus = SHORT1FROMMP(mp2))
		chainThreads.foreach(
		    idForeachPmWindowThreadSetForegroundPriority,
		    (void*)0);
	else
		chainThreads.foreach(
		    idForeachPmWindowThreadSetBackgroundPriority,
		    (void*)0);
	return (MRESULT)FALSE;
}


MRESULT processWindow::WMClose(MPARAM mp1, MPARAM mp2)
{	(void)mp1;
	(void)mp2;
	if (canCloseHierarchy())
	{       if (pMsgThread->structTreeWindows.iNumberOfElements == 1)
		{	//WinDestroyWindow(hwndFrame);
			return (MRESULT)FALSE;
		}
		else
		{	WinDestroyWindow(hwndFrame);
			return (MRESULT)TRUE;
		}
	}
	else
		return (MRESULT)TRUE;
}


Boolean windowTree::create(void)
{	if (!WinRegisterClass(pMsgThread->hab, (unsigned char*)pWindowClassName,
	   pMProcessWindowProc, classStyle, sizeof(this)))
		return FALSE;
	hwndFrame = WinCreateStdWindow(
	   getParentWindow() ? getParentWindow()->hwndClient : HWND_DESKTOP,
	   windowStyle, &frameFlags, (unsigned char*)pWindowClassName,
	   (unsigned char*)pTitelBarText, 0, (HMODULE)0L,
	   resourceId, &hwndClient);
#ifdef DEBUG
	if (!hwndFrame)
		fprintf(stderr, "Fehler bei `WinCreateStdWindow' = %x\n",
		   WinGetLastError(pMsgThread->hab));
#endif DEBUG
	hWindowDC = WinOpenWindowDC(hwndClient);
	WinSetWindowPtr(hwndClient, 0, this);
	WinSetWindowPos(hwndFrame,
	    HWND_TOP,
	    100, 100, 200, 200,
	    SWP_ACTIVATE | SWP_ZORDER | SWP_SHOW);
	return TRUE;
}


Boolean windowTree::initPost(void)
{	return TRUE;
}


MRESULT windowTree::WMPaint(MPARAM mp1, MPARAM mp2)
{	RECTL structRectl;
	HPS hps;

	(void)mp1;
	(void)mp2;
	hps = WinBeginPaint(hwndClient, (HPS)0, &structRectl);
	doPaint(hps, &structRectl);
	WinEndPaint(hps);
	return (MRESULT)TRUE;
}


void windowTree::doPaint(HPS hps, RECTL *pStructRectl)
{	(void)hps;
	(void)pStructRectl;
}


int windowTree::toBeCalledPre(unsigned int iMsg, void *pDummy)
{	switch (iMsg)
	{	case idForeachWindowTreeWinDestroyWindow:
		case idForeachWindowTreeCallInit:
		case idForeachWindowTreeCallDestructAll:
			return 0;
		case idForeachWindowTreeCanClose:
			if (!canClose())
				return -1;
			else
				return 0;
		default:
			return tree::toBeCalledPre(iMsg, pDummy);
	}
}


int windowTree::toBeCalledPost(unsigned int iMsg, void *pDummy)
{	switch (iMsg)
	{	case idForeachWindowTreeWinDestroyWindow:
			if (hwndFrame)
				WinDestroyWindow(hwndFrame);
			return 0;
		case idForeachWindowTreeCallInit:
			if (!init())
			{	delete this;
				return -1;
			}
			++*(unsigned int*)pDummy;
			return 0;
		case idForeachWindowTreeCallDestructAll:
			if ((*(unsigned int*)pDummy)--)
			{	destructAll();
				return 0;
			}
			else
				return -1;
		case idForeachWindowTreeCanClose:
			return 0;
		default:
			return tree::toBeCalledPost(iMsg, pDummy);
	}
}


Boolean windowTree::canClose(void)
{	return TRUE;
}


windowTree::~windowTree(void)
{	foreach(idForeachWindowTreeWinDestroyWindow, (void*)0);
	WinSetWindowPtr(hwndFrame, 0, 0);
}


pmThread::pmThread(pmProcess *pProcess, chain *pParent)
    :thread(pProcess, pParent)
{	hab = WinInitialize(0);
}


pmThread::pmThread(pmProcess *pProcess):thread(FALSE, pProcess)
{	hab = WinInitialize(0);
}


pmThread::~pmThread(void)
{	if (hab)
		WinTerminate(hab);
}


pmWindowThread::pmWindowThread(windowTree *pWindowNew)
    :pmThread((pmProcess*)pWindowNew->pMsgThread->pProcess
    ->getChainMember(idGetChainPmProcessThis),
    &pWindowNew->chainThreads)
{	pWindow = pWindowNew;
	iClassF = PRTYC_REGULAR;
	iValueF = 0;
	iClassB = PRTYC_IDLETIME;
	iValueB = 0;
}


Boolean pmWindowThread::initPre(void)
{	if (pWindow->bFocus)
		toBeCalledForeachElement(
		    idForeachPmWindowThreadSetForegroundPriority,
		    (void*)0);
	else
		toBeCalledForeachElement(
		    idForeachPmWindowThreadSetBackgroundPriority,
		    (void*)0);
	return pmThread::initPre();
}


int pmWindowThread::toBeCalledForeachElement(unsigned int iMsg, void *pDummy)
{      	switch (iMsg)
	{	default:
			return pmThread::toBeCalledForeachElement(iMsg, pDummy);
		case idForeachPmWindowThreadSetBackgroundPriority:
			DosSetPriority(PRTYS_THREAD,
			    iClassB,
			    iValueB - pTIB->tib_ptib2->tib2_ulpri,
			    pTIB->tib_ptib2->tib2_ultid);
			return 0;
		case idForeachPmWindowThreadSetForegroundPriority:
			DosSetPriority(PRTYS_THREAD,
			    iClassF,
			    iValueF - pTIB->tib_ptib2->tib2_ulpri,
			    pTIB->tib_ptib2->tib2_ultid);
			return 0;
	}
}

pmMsgThread::pmMsgThread(pmProcess *pProcess):pmThread(pProcess),
    structTreeWindows((tree*)0)
{	hmq = WinCreateMsgQueue(hab, 0);
}


Boolean pmMsgThread::create(void)
{	processWindow *pWindowTree;	// in this class only one window
					// will be opened
	if (!(pWindowTree
	   = new processWindow(this, 0, "")))
		return FALSE;
	else
		if (!pWindowTree->isSuccessfull())
		{	delete pWindowTree;
			return FALSE;
		}
	return TRUE;
}


void pmMsgThread::destruct()
{	structTreeWindows.foreach(idForeachWindowTreeWinDestroyWindow,
	    (void*)0);
}


Boolean pmMsgThread::initPost(void)
{       unsigned int iNumber = 0;

	structTreeWindows.foreach(idForeachWindowTreeCallInit, (void*)&iNumber);
	if (iNumber != structTreeWindows.iNumberOfElements)
	{	structTreeWindows.foreach(idForeachWindowTreeCallDestructAll,
		    (void*)&iNumber);
		return FALSE;
	}
	return TRUE;
}


void pmMsgThread::doSomething(void)
{	QMSG qmsg;

	while (WinGetMsg(hab, &qmsg, 0, 0, 0))
		WinDispatchMsg(hab, &qmsg);
}


void pmMsgThread::destructPre(void)
{       unsigned int iNumber = structTreeWindows.iNumberOfElements;

	structTreeWindows.foreach(idForeachWindowTreeCallDestructAll,
	    (void*)&iNumber);
}


pmMsgThread::~pmMsgThread(void)
{	if (hmq)
		WinDestroyMsgQueue(hmq);
}


Boolean pmProcess::create(void)
{	if (!(pProcessThread = new pmMsgThread(this)))
		return FALSE;
	else
		if (pProcessThread->isSuccessfull())
			return TRUE;
		else
		{	delete pProcessThread;
			pProcessThread = (thread*)0;
			return FALSE;
		}
}
