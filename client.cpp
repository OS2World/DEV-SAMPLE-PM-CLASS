#define INCL_DOSNMPIPES
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include "communication.h"


int main(int argc, char **argv)
{       (void)argc;
	(void)argv;
	HPIPE hPipe;
	APIRET iRet;
	unsigned long iAction;

	if (iRet = DosWaitNPipe(pPipeName, -1))
	{	fprintf(stderr,
		    "DosWaitNPipe() returned error code %u!\n",
		    iRet);
		exit(1);
	}
	if (iRet = DosOpen(pPipeName,
	    &hPipe,
	    &iAction,
	    0,
	    0,
	    0
	    | OPEN_ACTION_FAIL_IF_NEW
	    | OPEN_ACTION_OPEN_IF_EXISTS
	    ,
	    0
	    //| OPEN_FLAGS_WRITE_THROUGH
	    //| OPEN_FLAGS_FAIL_ON_ERROR
	    //| OPEN_FLAGS_NO_CACHE
	    //| OPEN_FLAGS_SEQUENTIAL
	    //| OPEN_FLAGS_NOINHERIT
	    | OPEN_SHARE_DENYNONE
	    | OPEN_ACCESS_WRITEONLY
	    ,
	    (EAOP2*)0))
	{	fprintf(stderr,
		    "DosOpen() returned error code %u!\n",
		    iRet);
		exit(1);
	}
	{       char acBuffer[1024];
		unsigned long iBytesRead;
		unsigned long iBytesWritten;


		while (!(iRet = DosRead(0,
		    acBuffer,
		    sizeof acBuffer,
		    &iBytesRead)
		    && iBytesRead))
		{	APIRET iRet;

			if (iRet = DosWrite(hPipe,
			    acBuffer,
			    iBytesRead,
			    &iBytesWritten))
			 {	fprintf(stderr,
				    "DosWrite returned error code %u\n!",
				    iRet);
				break;
			 }
		}
		if (iRet)			fprintf(stderr,
			    "DosRead returned error code %u\n!",
			    iRet);
	}
	return 0;
}