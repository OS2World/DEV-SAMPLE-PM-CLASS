#include "apfel.h"
#include "apfel.resource.h"
#include "communication.h"
#ifdef INCLUDESOURCE
#include "pmprocess.cc"
#endif


// The calculation thread if there are no servers
// The priority is for both foreground and background of class idle -
// caused of that this thread is not interactive
calcThread::calcThread(menuWindowTree *pWindowNew)
    :pmWindowThread(pWindowNew)
{	pWindow = pWindowNew;
	iClassF = PRTYC_IDLETIME;
	iValueF = 31;
	iClassB = PRTYC_IDLETIME;
	iValueB = 0;
}


void menuWindowTree::winInvalidate(unsigned int yi)
{	RECTL rcl;

	rcl.xLeft = 0;
	rcl.yTop = yi + 1;
	rcl.yBottom = yi;
	rcl.xRight = iSizeX;
	WinInvalidateRect(hwndClient, &rcl, FALSE);
}


// calculates a image
void calcThread::doSomething(void)
{	unsigned int xi, yi, it;
	//menuPmThread *pMsgThread = (menuPmThread*)pParent;
	double xl = pWindow->xa, yl = pWindow->ya, x, y;
	BITMAPINFO2 *pBitmapInfo;
	unsigned char *pData;
	unsigned char *pDataCompare;
	unsigned int iBitmapSize;

	pData = (unsigned char *)alloca(iBitmapSize = pWindow->iSizeX * sizeof(*pData));
	pDataCompare = (unsigned char *)alloca(iBitmapSize);
	assert(!((unsigned long)pData & 3));
	pBitmapInfo = (BITMAPINFO2 *)alloca(256*sizeof(RGB2)
	   + (unsigned int)((char*)&pBitmapInfo->ulCompression
	  - (char*)&pBitmapInfo->cbFix));
	memset(pBitmapInfo, 0, 256*sizeof(RGB2));
	{	RGB2 *pScan;
		unsigned short iColor;

		for (pScan = (RGB2*)&pBitmapInfo->ulCompression,
		   iColor = 0;
		   iColor < 256;
		   pScan++, iColor++)
		{	pScan->fcOptions = PC_EXPLICIT;
			pScan->bBlue = iColor;
		}
	}
	pBitmapInfo->cbFix = (unsigned int)((char*)&pBitmapInfo->ulCompression
	   - (char*)&pBitmapInfo->cbFix);
	assert(pBitmapInfo->cbFix > 0);
	pBitmapInfo->cx = pWindow->iSizeX;
	pBitmapInfo->cy = 1;
	pBitmapInfo->cPlanes = 1;
	pBitmapInfo->cBitCount = sizeof(*pData)*8;
	/*
	pBitmapInfo->ulCompression = BCA_UNCOMP;
	pBitmapInfo->cbImage = 0;
	pBitmapInfo->cxResolution = 0;
	pBitmapInfo->cyResolution = 0;
	pBitmapInfo->cclrUsed = 0;
	pBitmapInfo->cclrImportant = 0;
	pBitmapInfo->usUnits = 0;
	pBitmapInfo->usReserved = 0;
	pBitmapInfo->usRecording = BRA_BOTTOMUP;
	pBitmapInfo->usRendering = BRH_NOTHALFTONED;
	pBitmapInfo->ulColorEncoding = BCE_RGB;
	*/
	for (yi = 0, x = xl, y = yl;
	   yi < pWindow->iSizeY;
	   x = (xl -= pWindow->ydx), y = (yl += pWindow->xdx),
	   GpiSetBitmapBits(
		(unsigned long)pWindow->hpsBitmap,
		(LONG)yi,
		(LONG)1,
		(unsigned char*)pData,
		pBitmapInfo),
		// After some hours without any picture,
		// I wanted to have a look, wethere the bitmap will be changed
		// or not.
		// Thats why I read from the bitmap using the next function call
		// and I got a picture on the screen.
		// If I take this function call away - no more drawing action
		// will occure.
		// Is there anyone how can explain this?
	   GpiQueryBitmapBits(
		(unsigned long)pWindow->hpsBitmap,
		(LONG)yi,
		(LONG)1,
		(unsigned char*)pDataCompare,
		pBitmapInfo),
	   pWindow->winInvalidate(yi),
	   yi++)
	{	unsigned int iMaxColors_1 = pWindow->iMaxColors - 1;
		double dLimit = pWindow->dLimit;
		unsigned int iMaxIterations
		    = pWindow->iMaxIterations;
		// You don't need to write the following stuff in assembler!
		// Use a good compiler like EMX for OS/2!
		for (xi = 0,
		   memset(pData, 0, iBitmapSize);
		   xi < pWindow->iSizeX;
		   xi++,
		   y += pWindow->ydx, x += pWindow->xdx
		   )
		{	double y2 = y*y;
			double x_1_2 = x + 1.0;

			x_1_2 *= x_1_2;
			if (y2 + x_1_2 > 1.0/16.0)
			{	double r = x*x + y2;
				double s = r - x*0.5 + 1.0/16.0;
				if ((r*16 - 5.0)*s + 4*x > 1)
				{	double xit = x, yit = y;

					for (it = 0;
					    it < iMaxIterations;
					    it++)
					{	double x2 = xit*xit;
						double y2 = yit*yit;
						double r = y2 + x2;
						if (r > dLimit)
						{	pData[xi] = it
							    % iMaxColors_1 + 1;
							break;
						}
						r = 2*xit*yit + y;
						xit = x2 - y2 + x;
						yit = r;
					}
				}
			}
		}
	}
}


// If there are some pipe names start in server-client mode
// if not start the calculation thread  of class calcThread
Boolean menuWindowTree::startCalculation(void)
{       chainThreads.getMutualExclusiveAccess();
	if (pProcess->chainPipes.iNumberOfElements)
		if (pProcess->chainPipes.foreach(idForeachPipeNameMakeThread,
		    this))
		{	chainThreads.freeMutualExclusiveAccess();
			return FALSE;
		}
		else
		{	chainThreads.freeMutualExclusiveAccess();
			return TRUE;
		}
	else
	{	calcThread *pThread;

		pThread = new calcThread(this);
		if (pThread)
			if (pThread->isSuccessfull())
			{	pThread->startThread();
				chainThreads.freeMutualExclusiveAccess();
				return TRUE;
			}
			else
				delete pThread;
		chainThreads.freeMutualExclusiveAccess();
		return FALSE;
	}
}


// Kills all running threads
void menuWindowTree::stopCalculation(void)
{	chainThreads.getMutualExclusiveAccess();
	chainThreads.foreach(idForeachChainElementDestruct, (void*)0);
	chainThreads.freeMutualExclusiveAccess();
}


// The windowproc of this class
// some other message will be treated in the base classes
MRESULT menuWindowTree::windowProc(ULONG msg, MPARAM mp1, MPARAM mp2)
{	switch (msg)
	{	default:
			return processWindow::windowProc(msg, mp1, mp2);
		case WM_BUTTON2DOWN:
		{	WinPopupMenu(hwndClient,
			    hwndClient,
			    hwndMenu,
			    MOUSEMSG(&msg)->x,
			    MOUSEMSG(&msg)->y,
			    0,
			    PU_HCONSTRAIN
			    | PU_VCONSTRAIN
			    //| PU_MOUSEBUTTON2DOWN
			    | PU_KEYBOARD
			    | PU_MOUSEBUTTON1
			    | PU_MOUSEBUTTON2);
			return (MRESULT)TRUE;
		}
		case WM_BUTTON1DOWN:
		{	if (eZoomState == 3)
			{	drawZoomRectangle();
				WinEnableMenuItem(hwndMenu,
				    idResourceMenuItemZoomOld, FALSE);
				WinEnableMenuItem(hwndMenu,
				    idResourceMenuItemZoomNew, FALSE);
			}
			iZoomXE = iZoomXA = MOUSEMSG(&msg)->x;
			iZoomYE = iZoomYA = MOUSEMSG(&msg)->y;
			WinSetCapture(HWND_DESKTOP, hwndClient);
			eZoomState = 1;
			return (MRESULT)FALSE;
		}
		case WM_MOUSEMOVE:
		{	if (eZoomState == 0 || eZoomState == 3)
				return (MRESULT)FALSE;
			if (eZoomState == 2)
				drawZoomRectangle();
			iZoomXE = MOUSEMSG(&msg)->x;
			iZoomYE = MOUSEMSG(&msg)->y;
			if (iZoomXE == iZoomXA || iZoomYE == iZoomYA)
			{	eZoomState = 1;
				return (MRESULT)FALSE;
			}
			drawZoomRectangle();
			eZoomState = 2;
			return (MRESULT)FALSE;
		}
		case WM_BUTTON1UP:
		{	if (eZoomState != 2)
			{	if (eZoomState == 1)
				{	eZoomState = 0;
					WinSetCapture(HWND_DESKTOP, NULLHANDLE);
				}
				return (MRESULT)FALSE;
			}
			WinSetCapture(HWND_DESKTOP, NULLHANDLE);
			drawZoomRectangle();
			iZoomXE = MOUSEMSG(&msg)->x;
			iZoomYE = MOUSEMSG(&msg)->y;
			drawZoomRectangle();
			eZoomState = 3;
			WinEnableMenuItem(hwndMenu, idResourceMenuItemZoomOld,
			    TRUE);
			WinEnableMenuItem(hwndMenu, idResourceMenuItemZoomNew,
			    TRUE);
			return (MRESULT)FALSE;
		}
#ifdef undefined
	unsigned int eZoomState;	// Are the following Values valid
	//	0 means no valid zoom
	//	1 means first point selected
	//	2 has already moved, still moving
	//	3 means second point selected,
	int iZoomXA, iZoomYA, iZoomXE, iZoomYE;
#endif
		case WM_COMMAND:
			switch ((USHORT)mp1)
			{       default:
					return processWindow
					    ::windowProc(msg, mp1, mp2);
				case idResourceMenuItemZoomOld:
					if (eZoomState != 3)
						return (MRESULT)FALSE;
					stopCalculation();
					WinEnableMenuItem(hwndMenu,
					    idResourceMenuItemZoomOld,
					    FALSE);
					WinEnableMenuItem(hwndMenu,
					    idResourceMenuItemZoomNew,
					    FALSE);
					eZoomState = 0;
					calcZoomCoordinates();
					startCalculation();
					return (MRESULT)FALSE;				case idResourceMenuItemZoomNew:
				{	menuWindowTree *pWindowTree;

					if (eZoomState != 3)
						return (MRESULT)FALSE;
					if (!(pWindowTree
					    = new menuWindowTree(
					    (menuPmThread*)pMsgThread
					    ->getChainElementMember(
					    idGetChainElementMenuMsgThreadTreeThis)
					    ,
					    0,
					    "Apfel")))
						return (MRESULT)FALSE;
					else
						if (!pWindowTree->isSuccessfull())
						{	delete pWindowTree;
							return (MRESULT)FALSE;
						}
					pWindowTree->bCoordiantesValid = TRUE;
					pWindowTree->xa = xa;
					pWindowTree->xdx = xdx;
					pWindowTree->ya = ya;
					pWindowTree->ydx = ydx;
					pWindowTree->iMaxIterations = iMaxIterations;
					pWindowTree->dLimit = dLimit;
					pWindowTree->iZoomXA = iZoomXA;
					pWindowTree->iZoomYA = iZoomYA;
					pWindowTree->iZoomXE = iZoomXE;
					pWindowTree->iZoomYE = iZoomYE;
					//pWindowTree->eZoomState = eZoomState;
					if (!pWindowTree->init())
						delete pWindowTree;
					return (MRESULT)FALSE;
				}
			}
	}
}


void menuWindowTree::doPaint(HPS hps, RECTL *pStructRectl)
{	POINTL aPoint[4];

	aPoint[0].x = pStructRectl->xLeft;
	aPoint[0].y = pStructRectl->yBottom;
	aPoint[1].x = pStructRectl->xRight;
	aPoint[1].y = pStructRectl->yTop;
	aPoint[2].x = pStructRectl->xLeft;
	aPoint[2].y = pStructRectl->yBottom;
	aPoint[3].x = pStructRectl->xRight;
	aPoint[3].y = pStructRectl->yTop;
	if (eZoomState >= 2)
		drawZoomRectangle();
	GpiBitBlt(hps, hpsBitmap, 4, aPoint, ROP_SRCCOPY, BBO_OR);
	if (eZoomState >= 2)
		drawZoomRectangle();
}


Boolean menuWindowTree::initPost(void)
{       if (!processWindow::initPost())
		return FALSE;
	{	SIZEL structSize;

		structSize.cx = structSize.cy = 0;
		hZoomRectanglePS = GpiCreatePS(pMsgThread->hab,
		    hWindowDC,
		    &structSize,
		    PU_PELS | GPIA_ASSOC);
	}
	GpiSetMix(hZoomRectanglePS, FM_INVERT);
	hdcBitmap = DevOpenDC(pMsgThread->hab, OD_MEMORY, (unsigned char*)"*", 0, 0, 0);
	{	SIZEL size;

		size.cx = size.cy = 0;
		hpsBitmap = GpiCreatePS(pMsgThread->hab, hdcBitmap, &size,
		   PU_PELS | GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);
	}
	iSizeX = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
	iSizeY = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
	{	long alBmpFormats[2];
		BITMAPINFOHEADER2 structBmpInfoHeader;

		GpiQueryDeviceBitmapFormats(hpsBitmap, 2, alBmpFormats);
		memset(&structBmpInfoHeader, 0, sizeof(structBmpInfoHeader));
		structBmpInfoHeader.cbFix = sizeof(structBmpInfoHeader);
		structBmpInfoHeader.cx = iSizeX;
		structBmpInfoHeader.cy = iSizeY;
		structBmpInfoHeader.cPlanes = (USHORT)alBmpFormats[0];
		structBmpInfoHeader.cBitCount = (USHORT)alBmpFormats[1];
		hbm = GpiCreateBitmap(hpsBitmap,
		    &structBmpInfoHeader,
		    0,
		    (BYTE*)0,
		    (BITMAPINFO2*)0);
	}
	GpiSetBitmap(hpsBitmap, hbm);
	{	POINTL aPoint[4];

		aPoint[0].x = 0;
		aPoint[0].y = iSizeY;
		aPoint[1].x = iSizeX;
		aPoint[1].y = 0;
		aPoint[2].x = 0;
		aPoint[2].y = iSizeY;
		aPoint[3].x = iSizeX;
		aPoint[3].y = 0;
		GpiBitBlt(hpsBitmap, hpsBitmap, 4, aPoint, ROP_ZERO, BBO_OR);
	}
	// Wether the window is the first started or not
	// the values need initialization or not
	if (!bCoordiantesValid)
	{	xa = -2.0;
		xdx = 3.0 / (iSizeX - 1);
		ya = -xdx*(iSizeY - 1)*0.5;
		ydx = 0.0;
		iMaxIterations = 255;
		dLimit = 100.0;
	}
	else
		calcZoomCoordinates();
	{	HDC hdc;
		hdc = WinOpenWindowDC(HWND_DESKTOP);
		DevQueryCaps((HDC)hdc, (LONG)CAPS_COLORS, (LONG)1,
		    (LONG*)&iMaxColors);
		DevCloseDC(hdc);
	}
	//hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);
	hwndMenu = WinLoadMenu(hwndClient, (HMODULE)0, idResourceMenu);
	if (pProcess->chainPipes.iNumberOfElements)
	{	pabCalculated = new unsigned char[iSizeY];
		memset(pabCalculated, 0, iSizeY*sizeof*pabCalculated);
	}
	else
		pabCalculated = (unsigned char*)0;
	// start the calculation thread(s)
	return startCalculation();
}


// the matching counterpart for initPost()
void menuWindowTree::destructPre(void)
{       stopCalculation();
	if (pabCalculated)
		delete pabCalculated;
	GpiDeleteBitmap(hbm);
	DevCloseDC(hdcBitmap);
	GpiDestroyPS(hZoomRectanglePS);
	processWindow::destructPre();
}


// I think there is still a bug releated to the milestone problem
void menuWindowTree::drawZoomRectangle(void)
{	int iXa, iYa, iXe = iZoomXE,
	    iYe = iZoomYE, iXdx, iYdx, iXdy, iYdy;
	POINTL aStructPoint[5];

	iXa = 2*iZoomXA - iXe;
	iYa = 2*iZoomYA - iYe;
	iXdx = iXe - iXa;
	iYdx = iYe - iYa;
	iYdy = iXdx*(iSizeY - 1)/(iSizeX - 1)/2;
	iXdy = -iYdx*(iSizeY - 1)/(iSizeX - 1)/2;
	aStructPoint[0].x = iXa - iXdy; aStructPoint[0].y = iYa - iYdy;
	aStructPoint[1].x = iXe - iXdy; aStructPoint[1].y = iYe - iYdy;
	aStructPoint[2].x = iXe + iXdy; aStructPoint[2].y = iYe + iYdy;
	aStructPoint[3].x = iXa + iXdy; aStructPoint[3].y = iYa + iYdy;
	aStructPoint[4].x = iXa - iXdy; aStructPoint[4].y = iYa - iYdy;
	GpiMove(hZoomRectanglePS, aStructPoint);
	GpiPolyLine(hZoomRectanglePS, 5, aStructPoint);
}


Boolean menuWindowTree::initPre(void)
{	if (!processWindow::initPre())
		return FALSE;
	//frameFlags |= FCF_MENU;
	pWindowClassName = "menuWindowTree";
	return TRUE;
}


menuWindowTree::menuWindowTree(menuPmThread *pMsgThreadNew, int idResource,
    char *pTitle):processWindow(pMsgThreadNew, idResource, pTitle), oMtx()
{	pMsgThread = pMsgThreadNew;
	pProcess = pMsgThread->pProcess;
}


menuWindowTree::~menuWindowTree(void)
{
}


// I think there is still a bug releated to a milestone problem
void menuWindowTree::calcZoomCoordinates(void)
{	int iXa, iYa, iXe = iZoomXE, iYe = iZoomYE, iXdx, iYdx, iXdy, iYdy;
	double dXaNew, dYaNew, dxdxNew, dydxNew;

	iXa = 2*iZoomXA - iXe;
	iYa = 2*iZoomYA - iYe;
	iXdx = iXe - iXa;
	iYdx = iYe - iYa;
	iYdy = iXdx*iSizeY/(iSizeX - 1)/2;
	iXdy = -iYdx*iSizeY/(iSizeX - 1)/2;
	iXa -= iXdy; iYa -= iYdy;
	iXe -= iXdy; iYe -= iYdy;
	dXaNew = xa + iXa*xdx - iYa*ydx;
	dYaNew = ya + iXa*ydx + iYa*xdx;
	dxdxNew = (xa + iXe*xdx - iYe*ydx - dXaNew)/(iSizeX - 1);
	dydxNew = (ya + iXe*ydx + iYe*xdx - dYaNew)/(iSizeX - 1);
	xa = dXaNew;
	ya = dYaNew;
	xdx = dxdxNew;
	ydx = dydxNew;
}


menuPmThread::menuPmThread(menuPmProcess *pProcessNew)
    :pmMsgThread(pProcessNew)
{	pProcess = pProcessNew;
}


Boolean menuPmThread::create(void)
{	menuWindowTree *pWindowTree;

	if (!(pWindowTree
	   = new menuWindowTree(this, 0, "Apfel")))
		return FALSE;
	else
		if (!pWindowTree->isSuccessfull())
		{	delete pWindowTree;
			return FALSE;
		}
	return TRUE;
}


Boolean menuPmProcess::create(void)
{	if (argc > 1)
	{	FILE *pFile;
		char acLine[1024];

		if (!(pFile = fopen(argv[1], "r")))
		{	fprintf(stderr, "Cannot open %s!\n", argv[1]);
			return FALSE;
		}
		acLine[sizeof acLine - 1] = 0;
		// reading of the file
		// create a object of class pipeName for every line in the file
		while (fgets(acLine, sizeof acLine - 1, pFile))
		{	pipeName *p;
			char *pPos;

			if (pPos = strchr(acLine, '\n'))
				*pPos = 0;
			if (p = new pipeName(acLine, &chainPipes))
				if (p->isSuccessfull())
					continue;
				else
					delete p;
			fprintf(stderr, "Not enough memory!\n");
			return FALSE;
		}
		fclose(pFile);
	}
	if (!new menuPmThread(this))
		return FALSE;
	else
		if (!pProcessThread->isSuccessfull())
		{	delete pProcessThread;
			return FALSE;
		}
		else
			return TRUE;
}


int pipeName::toBeCalledForeachElement(unsigned int iMsg, void *pData)
{	switch (iMsg)
	{	default:
			return stringChainElement
			    ::toBeCalledForeachElement(iMsg, pData);
		case idForeachPipeNameMakeThread:
		{       clientThread *pThread;

			if (!(pThread = new clientThread((menuWindowTree*)pData,
			    pString)))
				return -1;
			else
				if (!pThread->isSuccessfull())
				{	delete pThread;
					return -1;
				}
			pThread->startThread();
			return 0;
		}
	}
}


clientThread::clientThread(menuWindowTree *pWindowNew, char *pPipeName)
    :pmWindowThread(pWindowNew)
{	unsigned long iAction;

	pWindow = pWindowNew;
	//wait for a listening pipe available
	if (DosWaitNPipe((unsigned char*)pPipeName, -1)
	    || DosOpen(pPipeName, &hPipe, &iAction, 0, 0,
	    OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
	    OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE,
	    (EAOP2*)0))
	{	setNoSuccess();
		hPipe = (HPIPE)0;
		return;
	}
}

clientThread::~clientThread(void)
{	if (hPipe)
		DosClose(hPipe);
}


void clientThread::doSomething(void)
{	BITMAPINFO2 *pBitmapInfo;
	unsigned char *pData;
	unsigned char *pDataCompare;
	unsigned int iBitmapSize;

	pData = (unsigned char *)alloca(iBitmapSize = pWindow->iSizeX * sizeof(*pData));
	pDataCompare = (unsigned char *)alloca(iBitmapSize);
	assert(!((unsigned long)pData & 3));
	pBitmapInfo = (BITMAPINFO2 *)alloca(256*sizeof(RGB2)
	   + (unsigned int)((char*)&pBitmapInfo->ulCompression
	  - (char*)&pBitmapInfo->cbFix));
	memset(pBitmapInfo, 0, 256*sizeof(RGB2));
	{	RGB2 *pScan;
		unsigned short iColor;

		for (pScan = (RGB2*)&pBitmapInfo->ulCompression,
		   iColor = 0;
		   iColor < 256;
		   pScan++, iColor++)
		{	pScan->fcOptions = PC_EXPLICIT;
			pScan->bBlue = iColor;
		}
	}
	pBitmapInfo->cbFix = (unsigned int)((char*)&pBitmapInfo->ulCompression
	   - (char*)&pBitmapInfo->cbFix);
	assert(pBitmapInfo->cbFix > 0);
	pBitmapInfo->cx = pWindow->iSizeX;
	pBitmapInfo->cy = 1;
	pBitmapInfo->cPlanes = 1;
	pBitmapInfo->cBitCount = sizeof(*pData)*8;

	// make sure that only one thread has access to pWindow->iLine
	// and pWindow->pabCalculated
	pWindow->oMtx.getMutualExclusiveAccess();
	while (1)
	{	unsigned int iLine;
		structRequest structR;
		unsigned long iBytes;

		if (pWindow->iLine < pWindow->iSizeY)
			iLine = pWindow->iLine++;
		else
		{	unsigned char *pScan, *pMax;

			for (pScan = pWindow->pabCalculated,
			    pMax = pScan + pWindow->iSizeY;
			    pScan < pMax;
			    pScan++)
				if (!*pScan)
				{	iLine = pScan - pWindow->pabCalculated;
					break;
				}
			if (pScan == pMax)
			{	pWindow->oMtx.freeMutualExclusiveAccess();
				return;
			}
		}
		pWindow->oMtx.freeMutualExclusiveAccess();
		structR.iSizeX = pWindow->iSizeX;
		structR.iMaxIterations = pWindow->iMaxIterations;
		structR.iMaxColors = pWindow->iMaxColors;
		structR.dLimit = pWindow->dLimit;
		structR.dXa = pWindow->xa - pWindow->ydx*iLine;
		structR.dYa = pWindow->ya + pWindow->xdx*iLine;
		structR.dXdx = pWindow->xdx;
		structR.dYdx = pWindow->ydx;
		if (DosWrite(hPipe, &structR, sizeof structR, &iBytes)
		    || iBytes != sizeof structR)
			return;
		if (DosRead(hPipe, pData, pWindow->iSizeX*sizeof*pData,
		    &iBytes)
		    || iBytes != pWindow->iSizeX*sizeof*pData)
			return;
		pWindow->oMtx.getMutualExclusiveAccess();
		do
		{	GpiSetBitmapBits(
			    (unsigned long)pWindow->hpsBitmap,
			    (LONG)iLine,
			    (LONG)1,
			    (unsigned char*)pData,
			    pBitmapInfo);
		// After some hours without any picture,
		// I wanted to have a look, wethere the bitmap will be changed
		// or not.
		// Thats why I read from the bitmap using the next function call
		// and I got a picture on the screen.
		// If I take this function call away - no more drawing action
		// will occure.
		// Is there anyone how can explain this?
			GpiQueryBitmapBits(
			    (unsigned long)pWindow->hpsBitmap,
			    (LONG)iLine,
			    (LONG)1,
			    (unsigned char*)pDataCompare,
			    pBitmapInfo);
		// I've included this, caused of that calling one time
		// above two functions caused no change in the bitmap
		// in the first one or two lines.
		// I've checked this!
		// May be the display.dll is a little bit buggy
		} while (memcmp(pData,
		    pDataCompare,
		    pWindow->iSizeX*sizeof*pData));
		pWindow->winInvalidate(iLine);
		pWindow->pabCalculated[iLine] = TRUE;
	}
}


int main(int argc, char **argv)
{      menuPmProcess *p = new menuPmProcess(argc, argv);

	if (p)
		if (p->isSuccessfull())
		{	p->run();
			delete p;
		}
		else
			delete p;
	DosExit(EXIT_PROCESS, 0);
	return 0;
}
