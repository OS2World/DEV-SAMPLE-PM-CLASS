extern "C"
{
#include <stdlib.h>
};


#include "chain.h"
#ifdef INCLUDESOURCE
#include "object.cc"
#endif


chain::chain(void):mtxObject()
{	pFirst = pLast = (chainElement*)0;
	iNumberOfElements = 0;
}


chain::~chain(void)
{       foreach(idForeachChainElementDestruct, (void*)0);
}


int chain::foreach(unsigned int iMsg, void *pDummy)
{	chainElement *pScan, *pSucc;
	int ret;

	getMutualExclusiveAccess();
	for ((pScan = pFirst) ? (pSucc = pScan->pSucc)
	    : (pSucc = (chainElement*)0);
	    pScan;
	    (pScan = pSucc) ? (pSucc = pSucc->pSucc)
	    : (pSucc = (chainElement*)0))
		if (ret = pScan->toBeCalledForeachElement(iMsg, pDummy))
		{	freeMutualExclusiveAccess();
			return ret;
		}
	freeMutualExclusiveAccess();
	return 0;
}


/*
BubbleSort
*/
void chain::bubbleSort(unsigned int iMsg)
{	Boolean bSorted;
	unsigned int iIndex, iMax;

	getMutualExclusiveAccess();
#ifdef DEBUG
	if (pFirst)
		pFirst->checkConsistence();
	else
		assert(!iNumberOfElements);
#endif
	for (iMax = iNumberOfElements, bSorted = TRUE;
	    iMax > 1;
	    iMax--, bSorted = TRUE)
	{	chainElement *pScan;

		for (pScan = pFirst, iIndex = 1;
		    iIndex < iMax;
		    pScan = pScan->pSucc, iIndex++)
			if (!pScan->laterThenThis(pScan->pSucc, iMsg))
			{	pScan->exchange(pScan->pSucc);
				pScan = pScan->pPrev;
				bSorted = FALSE;
			}
		if (bSorted)
			break;
	}
#ifdef DEBUG
	if (pFirst)
		pFirst->checkConsistence();
	else
		assert(!iNumberOfElements);
#endif
	freeMutualExclusiveAccess();
}


void chainElement::exchange(chainElement *pElement)
{	chainElement *pHelp;

	assert(pParent);
	assert(pElement->pParent);
	getMutualExclusiveAccess();
	pElement->getMutualExclusiveAccess();
#ifdef DEBUG
	pParent->pFirst->checkConsistence();
#endif
	if (this == pElement)
		return;
	if (pParent != pElement->pParent)
	{	chain *pHelp;

		if (pParent->pFirst == this)
			pParent->pFirst = pElement;
		if (pParent->pLast == this)
			pParent->pLast = pElement;
		if (pElement->pParent->pFirst == pElement)
			pElement->pParent->pFirst = this;
		if (pElement->pParent->pLast == pElement)
			pElement->pParent->pLast = this;
		pHelp = pParent;
		pParent = pElement->pParent;
		pElement->pParent = pHelp;
	}
	else
	{	if (pParent->pFirst == this)
			pParent->pFirst = pElement;
		else
			if (pParent->pFirst == pElement)
				pParent->pFirst = this;
		if (pParent->pLast == this)
			pParent->pLast = pElement;
		else
			if (pParent->pLast == pElement)
				pParent->pLast = this;
	}
	if (pPrev != pElement && pSucc != pElement)
	{	pHelp = pPrev;
		pPrev = pElement->pPrev;
		pElement->pPrev = pHelp;
		pHelp = pSucc;
		pSucc = pElement->pSucc;
		pElement->pSucc = pHelp;
#ifdef DEBUG
		pParent->pFirst->checkConsistence();
#endif
	}
	else
		if (pPrev == pElement)
		{	pPrev = pElement->pPrev;
			pElement->pSucc = pSucc;
			pSucc = pElement;
			pElement->pPrev = this;
		}
		else
			if (pSucc == pElement)
			{	pSucc = pElement->pSucc;
				pElement->pPrev = pPrev;
				pPrev = pElement;
				pElement->pSucc = this;
			}
	if (pSucc)
		pSucc->pPrev = this;
	if (pPrev)
		pPrev->pSucc = this;
	if (pElement->pSucc)
		pElement->pSucc->pPrev = pElement;
	if (pElement->pPrev)
		pElement->pPrev->pSucc = pElement;
#ifdef DEBUG
	pParent->pFirst->checkConsistence();
#endif
	freeMutualExclusiveAccess();
	pElement->freeMutualExclusiveAccess();}


int chainElement::toBeCalledForeachElement(unsigned int iMsg, void *pDummy)
{	switch (iMsg)
	{	case idForeachChainElementDestruct:
			delete this;
			return 0;
		case idForeachChainElementMove:
			move((chain*)pDummy);
			return 0;
		default:
			fprintf(stderr,
			   "Unknown Message Type (%u) to function \"chainElement::toBeCalledForeachElement\"\n",
			   iMsg);
			abort();
			return 0;
	}
}


#ifdef undefined
Boolean chainElement::isEqual(void *pDummy)
{	(void)pDummy;

	abort();
	return FALSE;
}
#endif


chainElement::chainElement(void):mtxObject()
{	pParent = (chain*)0;
	pPrev = pSucc = (chainElement*)0;
}


void chainElement::insertSorted(chain *pParentNew, unsigned int iMsg)
{	assert(!pParent);
	assert(pParentNew);
	pParent = pParentNew;
	getMutualExclusiveAccess();
	if (!pParentNew->pLast)
	{	pParentNew->pFirst = pParentNew->pLast = this;
		pPrev = pSucc = (chainElement*)0;
	}
	else
	{	chainElement *pScan;

		for (pScan = pParent->pFirst; pScan; pScan = pScan->pSucc)
			if (laterThenThis(pScan, iMsg))
			{	pSucc = pScan;
				pPrev = pScan->pPrev;
				pScan->pPrev = this;
				if (pScan == pParent->pFirst)
					pParent->pFirst = pScan;
				break;
			}
		if (!pScan)
		{	pPrev = pParent->pLast;
			pParent->pLast->pSucc = this;
			pParent->pLast = this;
		}
	}
	pParent->iNumberOfElements++;
#ifdef DEBUG
	pParent->pFirst->checkConsistence();
#endif
	freeMutualExclusiveAccess();
}


chainElement::chainElement(chain *pParentNew):mtxObject()
{	assert(!pPrev);
	assert(!pSucc);
	assert(!pParent);
	if (!(pParent = pParentNew))
	{	pPrev = pSucc = (chainElement*)0;
		return;
	}
	else
		getMutualExclusiveAccess();
	if (!pParentNew->pLast)
		pParentNew->pFirst = pParentNew->pLast = this;
	else
	{	pParentNew->pLast->pSucc = this;
		pPrev = pParentNew->pLast;
		pParentNew->pLast = this;
	}
	pParent->iNumberOfElements++;
#ifdef DEBUG
	pParent->pFirst->checkConsistence();
#endif /* DEBUG */
	freeMutualExclusiveAccess();
}


chainElement::~chainElement(void)
{	if (!pParent)
		return;
	getMutualExclusiveAccess();
#ifdef DEBUG	pParent->pFirst->checkConsistence();
#endif /* DEBUG */
	if (pPrev)
		pPrev->pSucc = pSucc;
	else
		pParent->pFirst = pSucc;
	if (pSucc)
		pSucc->pPrev = pPrev;
	else
		pParent->pLast = pPrev;
	pParent->iNumberOfElements--;
	freeMutualExclusiveAccess();
}


void chainElement::move(chain *pChain)
{	if (pParent == pChain)
		return;

	getMutualExclusiveAccess();
	if (pParent)
	{	if (pPrev)
			pPrev->pSucc = pSucc;
		else
			pParent->pFirst = pSucc;
		if (pSucc)
			pSucc->pPrev = pPrev;
		else
			pParent->pLast = pPrev;
		pParent->iNumberOfElements--;
		pParent->freeMutualExclusiveAccess();
	}
	if (pChain)
	{	pParent = pChain;		    // container
		pParent->getMutualExclusiveAccess();
		if (!pChain->pLast)
		{	pChain->pFirst = pChain->pLast = this;
			pPrev = pSucc = (chainElement*)0;
		}
		else
		{
			(pPrev = pChain->pLast)->pSucc = this;
			pChain->pLast = this;
			pSucc = (chainElement*)0;
		}
		pParent->iNumberOfElements++;
	}
	freeMutualExclusiveAccess();
}


#ifdef DEBUG


void chainElement::checkConsistence(void)
{	getMutualExclusiveAccess();
	if (pPrev)
	{	assert(pParent->pFirst != this);
		assert(pPrev->pSucc == this);
		assert(pParent == pPrev->pParent);
	}
	else
	{	chainElement *pScan;
		unsigned int iNumberOfElements;

		assert(pParent->pFirst == this);
		for (pScan = this, iNumberOfElements = 0;
		    pScan;
		    pScan = pScan->pSucc)
			iNumberOfElements++;
		assert(iNumberOfElements == pParent->iNumberOfElements);
	}
	if (pSucc)
	{	assert(pParent->pLast != this);
		assert(pSucc->pPrev == this);
		assert(pParent == pSucc->pParent);
		pSucc->checkConsistence();
	}
	else
		assert(pParent->pLast == this);
	freeMutualExclusiveAccess();
}


#endif /* DEBUG */


Boolean chainElement::laterThenThis(chainElement *pElement, unsigned int iMsg)
{	(void)pElement;
	(void)iMsg;
	return FALSE;
}


#ifdef undefined
Boolean chainElement::isLocked(void)
{	return pParent ? pParent->isLocked() : FALSE
	    || mtxObject::isLocked();
}
#endif undefined


void chainElement::getMutualExclusiveAccess(void)
{       if (pParent)
		pParent->getMutualExclusiveAccess();
	mtxObject::getMutualExclusiveAccess();
}


void chainElement::freeMutualExclusiveAccess(void)
{	mtxObject::freeMutualExclusiveAccess();
	if (pParent)
		pParent->freeMutualExclusiveAccess();
}

