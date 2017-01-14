#include "server.h"


serverMainThread::serverMainThread(serverProcess *pProcessNew)
    :thread(FALSE, pProcessNew)
{	pServerProcess = pProcessNew;
}


void serverMainThread::doSomething(void)
{       serverThread *pThread;

	if (!(pThread = new serverThread(pServerProcess)))
		return;
	else
		if (!pThread->isSuccessfull())
		{	delete pThread;
			return;
		}
	pThread->startThread();
	while (1)
		DosSleep(1000*60);
}


serverThread::serverThread(serverProcess *pProcessNew)
    :thread(TRUE, pProcessNew)
{	pServerProcess = pProcessNew;
}


Boolean serverThread::create(void)
{	APIRET iRet;

	if (iRet = DosCreateNPipe((unsigned char*)pServerProcess->pPipeName,
	    &hPipe,
	    NP_NOWRITEBEHIND | NP_NOINHERIT | NP_ACCESS_DUPLEX,
	    NP_WAIT | NP_TYPE_MESSAGE | NP_READMODE_MESSAGE | NP_UNLIMITED_INSTANCES,
	    1024*16,
	   1024*16,
	   0))
	{	fprintf(stderr,
		    "DosCreateNPipe() returned error code %u!\n",
		    iRet);
		return FALSE;
	}
	DosSetPriority(PRTYS_THREAD,
	    PRTYC_IDLETIME,
	    -pTIB->tib_ptib2->tib2_ulpri,
	    pTIB->tib_ptib2->tib2_ultid);
	return TRUE;
}


void serverThread::destruct(void)
{	DosClose(hPipe);
}


Boolean serverThread::calculate(structRequest *pStructR)
{       unsigned int xi;
	double x, y;
	unsigned int iSize;
	unsigned long iBytesWritten;
	APIRET iRet;
	unsigned char *paiData;

	if (pStructR->iSizeX > 16384)
		return FALSE;
	if (!(paiData
	    = (unsigned char*)alloca(iSize = pStructR->iSizeX*sizeof*paiData)))
		return FALSE;
	(void)memset((void*)paiData, 0, iSize);
	for (xi = 0, x = pStructR->dXa, y = pStructR->dYa;
	   xi < pStructR->iSizeX;
	   xi++, y += pStructR->dYdx, x += pStructR->dXdx)
	{	double y2 = y*y;
		double x_1_2 = x + 1.0;

		x_1_2 *= x_1_2;
		if (y2 + x_1_2 > 1.0/16.0)
		{	double r = x*x + y2;
			double s = r - x*0.5 + 1.0/16.0;
			if ((r*16 - 5.0)*s + 4*x > 1)
			{	double xit = x, yit = y;
				unsigned short it;

				for (it = 0;
				    it < pStructR->iMaxIterations;
				    it++)
				{	double x2 = xit*xit;
					double y2 = yit*yit;
					double r = y2 + x2;

					if (r > pStructR->dLimit)
					{	paiData[xi]
						    = it
						    % (pStructR->iMaxColors - 1)
						    + 1;
						break;
					}
					r = 2*xit*yit + y;
					xit = x2 - y2 + x;
					yit = r;
				}
			}
		}
	}
	if (iRet = DosWrite(hPipe, paiData, iSize, &iBytesWritten))
	{	fprintf(stderr,
		    "DosRead() returned error code %u!\n",
		    iRet);
		return FALSE;
	}
	if (iBytesWritten != iSize)
        {	fprintf(stderr,
		    "Wrong number of bytes written (%u)!\n",
		    iBytesWritten);
		return FALSE;
	}
	return TRUE;
}


void serverThread::doSomething(void)
{       structRequest structR;
	unsigned long iBytesRead;
	unsigned int iRet;

	if (iRet = DosConnectNPipe(hPipe))
	{	fprintf(stderr,
		    "DosConnectNPipe() returned error code %u!\n",
		    iRet);
		return;
	}
	{	serverThread *pThread = new serverThread(pServerProcess);

		if (pThread)
			if (pThread->isSuccessfull())
				pThread->startThread();
			else
			{	delete pThread;
				fprintf(stderr,
				    "Could not successfull create thread!\n");
			}
		else
			fprintf(stderr, "Could not create thread!\n");
	}
	while (!(iRet = DosRead(hPipe,
	    &structR,
	    sizeof structR,
	    &iBytesRead))
	    && iBytesRead == sizeof structR)
		if (!calculate(&structR))
			break;
	if (iRet)
		fprintf(stderr,
		    "DosRead() returned error code %u!\n",
		    iRet);
	else
		if (iBytesRead != sizeof structR)
			fprintf(stderr,
			    "Invalid number of bytes in request!\n");
	if (iRet = DosDisConnectNPipe(hPipe))
		fprintf(stderr,
		    "DosDisConnectNPipe() returned error code %u!\n",
		    iRet);
}


serverProcess::serverProcess(int argc, char **argv):process(argc, argv)
{	if (argc == 2)
		pPipeName = argv[1];
	else
	{	fprintf(stderr, "Usage: %s \\pipe\\myPipeName", argv[0]);
		setNoSuccess();
	}
}


Boolean serverProcess::create(void)
{	if (!(pProcessThread = new serverMainThread(this)))
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


int main(int argc, char **argv)
{	serverProcess *pProcess = new serverProcess(argc, argv);

	if (pProcess)
		pProcess->run();
	delete pProcess;
	DosExit(EXIT_PROCESS, 0);
	return 0;
}

