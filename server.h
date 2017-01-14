#define INCL_DOSNMPIPES
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <string.h>
#include "process.h"
#include "communication.h"


class serverProcess;


class serverThread:public thread
{       public:
	serverProcess *pServerProcess;
	HPIPE hPipe;
	serverThread(serverProcess *pProcessNew);
	//virtual ~serverThread(void)
	virtual Boolean create(void);
	Boolean calculate(structRequest *pStructR);
	virtual void doSomething(void);
	virtual void destruct(void);
};


class serverMainThread:public thread
{	public:
	serverProcess *pServerProcess;
	serverMainThread(serverProcess *pProcess);
	virtual void doSomething(void);
};


class serverProcess:public process
{	public:
	char *pPipeName;
	serverProcess(int argc, char **argv);
	virtual Boolean create(void);
};
