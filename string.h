#include "object.h"
#include <string.h>
#include <stdlib.h>
#include <mem.h>


class string:public virtual object
{       public:
	char *pString;
	unsigned int iStrLen;
	string(char *pStringNew)
	{       iStrLen = strlen(pStringNew);
		if (!(pString = (char*)malloc(iStrLen + 1)))
			setNoSuccess();
		else
			memcpy(pString, pStringNew, iStrLen + 1);
	}
	virtual ~string(void)
	{       if (pString)
			free(pString);
	}
	Boolean concat(char *pStringNew)
	{       unsigned int iStrLenNew = strlen(pStringNew);
		char *pHelp;

		if (!(pHelp = (char*)realloc(pString, iStrLen + iStrLenNew + 1)))
			return FALSE;
		else
		{       pString = pHelp;
			memcpy(pString + iStrLen, pStringNew, iStrLenNew + 1);
			iStrLen += iStrLenNew;
			return TRUE;
		}
	}
};


#define idForeachStringChainElementLast idForeachChainElementLast
class stringChainElement:public chainElement, public string
{	public:
	stringChainElement(char *pString, chain *pParent)
	    :chainElement(pParent), string(pString)
	{
	}
};
