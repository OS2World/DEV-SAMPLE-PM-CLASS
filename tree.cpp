#include "tree.h"
#ifdef INCLUDESOURCE
#include "chain.cc"
#endif


int tree::toBeCalledForeachElement(unsigned int iMsg, void *pDummy)
{	int ret;

	if (iMsg <= idForeachChainElementLast)
		return chainElement::toBeCalledForeachElement(iMsg, pDummy);
	switch (ret = toBeCalledPre(iMsg, pDummy))
	{	case 0:
			break;
		case -1:
			return 0;
		default:
			return ret;
	}
	if (ret = foreach(iMsg, pDummy))
		return ret;
	return toBeCalledPost(iMsg, pDummy);
}


int tree::toBeCalledPre(unsigned int iMsg, void *pDummy)
{	switch (iMsg)
	{	case idForeachTreeDestructPostChilds:
			return 0;
		case idForeachTreeSort:
			bubbleSort((unsigned int)pDummy);
			return 0;
		default:
			if (iMsg > idForeachTreeLast)
				abort();
			return 0;
	}
}


int tree::toBeCalledPost(unsigned int iMsg, void *pDummy)
{       (void)pDummy;
	switch (iMsg)
	{	case idForeachTreeDestructPostChilds:
			delete this;
			return 0;
		default:
			return 0;
	}
}


tree::~tree(void)
{
}


tree::tree(chain *pParent):chainElement(pParent), chain()
{
}


void tree::bubbleSortTree(unsigned int iMsg)
{	toBeCalledForeachElement(idForeachTreeSort, (void*)iMsg);
}
