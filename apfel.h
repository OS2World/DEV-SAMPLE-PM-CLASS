#ifndef MENUPMPROCESS_H
#define MENUPMPROCESS_H

#define INCL_PM
#define INCL_GPIBITMAPS
#define INCL_DOSNMPIPES
extern "C"
{
#include <memory.h>
#include <stdio.h>
};
#include "pmProcess.h"
#include "string.h"


class menuPmThread;
class menuWindowTree;
class menuPmProcess;
class calcThread;
class clientThread;
class pipeName;


// Mandelbrot calculating window
class menuWindowTree:public processWindow
{       public:
	// number of pixels
	int iSizeX, iSizeY;
	//HDC hdc;
	HDC hdcBitmap;
	HPS hpsBitmap;
	HBITMAP hbm;
	// to make shure, that only one thread accesses the next two variables
	mtxObject oMtx;
	// array of Booleans, which line is already calculated
	unsigned char *pabCalculated;
	// which line is the next for the next free server
	unsigned int iLine;
	// if this is true, the window has been started with coordinates
	// from a zoom - the coordinates will not be initialized in one
	// the initPre() or initPost() function
	Boolean bCoordiantesValid;	// Are the following Values valid
	// xdx is the x value difference between two horizontal pixels
	// ydx is the y value difference between two horizontal pixels
	// The absolute difference between two pixels is always
	// the same for both horizontal or vertical pixels
	// The x value difference between two vertical pixels is -ydx
	// The y value difference between two vertical pixels is xdx
	// dLimit is the absolute value^2 of the complex value pair (x,y)
	// which must be reached
	double xa, ya, xdx, ydx, dLimit;
	unsigned int eZoomState;
	//	0 means no valid zoom
	//	1 means first point selected
	//	2 has already moved, still moving
	//	3 means second point selected,
	int iZoomXA, iZoomYA, iZoomXE, iZoomYE;
	unsigned int iMaxIterations, iMaxColors;
	HPS hZoomRectanglePS;
	HWND hwndMenu;
	void winInvalidate(unsigned int yi);
	virtual void doPaint(HPS hps, RECTL *pStructRectl);
	virtual void destructPre(void);
	void drawZoomRectangle(void);
	//calcThread *pCalcThread;
	menuPmProcess *pProcess;
	menuPmThread *pMsgThread;
	//animateThread *pAnimateThread;
	//fullScreen *pFullScreen;
	virtual MRESULT windowProc(ULONG msg, MPARAM mp1, MPARAM mp2);
	menuWindowTree(menuPmThread *pMsgThreadNew, int idResource,
	    char *pTitle);
	virtual ~menuWindowTree(void);
	virtual Boolean initPost(void);
	virtual Boolean initPre(void);
	void calcZoomCoordinates(void);
	Boolean startCalculation(void);
	void stopCalculation(void);
};


// this thread does the calculation of the mandelbrot set in one window
class calcThread:public pmWindowThread
{       public:
	//menuPmThread *pMsgThread;
	menuWindowTree *pWindow;
	//calcThread(menuPmThread *pParentNew);
	calcThread(menuWindowTree *pWindowNew);
	inline void winInvalidate(unsigned int yi);
	//virtual ~calcThread(void);
	virtual void doSomething(void);
};


// a thread which sends requests to the server and wait for the replies
// There is one such thread for every server available
class clientThread:public pmWindowThread
{	public:
	// handle for pipe to server
	HPIPE hPipe;
	menuWindowTree *pWindow;
	clientThread(menuWindowTree *pWindowNew, char *pPipeName);
	virtual ~clientThread(void);
	virtual void doSomething(void);
};


// The message thread
#define idGetChainElementMenuMsgThreadTreeThis \
    (idGetChainElementPmMsgThreadTreeLast + 1)
class menuPmThread:public pmMsgThread
{       public:
	menuPmProcess *pProcess;
	menuPmThread(menuPmProcess *pProcess);
	virtual Boolean create(void);
	//virtual Boolean initPost(void);
	virtual void *getChainElementMember(unsigned int iMsg)
	{	switch (iMsg)
		{	default:
				return pmMsgThread::getChainElementMember(iMsg);
			case idGetChainElementMenuMsgThreadTreeThis:
				return (void*)this;
		}
	}
};


// This class will be called, to create threads for all serves
// which are available.
// Se implementation of toBeCalledForeachElement
// The parent of such element is menuPmProcess.chainPipes
#define idForeachPipeNameMakeThread (idForeachStringChainElementLast + 1)
class pipeName:public stringChainElement
{	public:
	pipeName(char *pString, chain *pParent)
	    :stringChainElement(pString, pParent)
	{
	}
	virtual int toBeCalledForeachElement(unsigned int iMsg, void *pDummy);
};


// The process
class menuPmProcess:public pmProcess
{       public:
	// contains objects of type stringChainElement
	// This are names of pipes provided by servers for calculating
	// on all machines available.
	// This names will be read from a file. The name of the file
	// should be passed on the commandline!
	// You should start on every OS/2 machine in the net
	// a server by passing a unique pipe name.
	// A pipe name must include the prefix "\pipe\".
	chain chainPipes;
	virtual Boolean create(void);
	menuPmProcess(int argc, char **argv):pmProcess(argc, argv), chainPipes()
	{
	}
};


#endif !MENUPMPROCESS_H
