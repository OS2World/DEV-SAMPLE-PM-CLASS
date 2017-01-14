#ifndef TREE_H
#define TREE_H


#include "chain.h"


#define idForeachTreeDestructPostChilds (idForeachChainElementLast + 1)
#define idForeachTreeSort (idForeachChainElementLast + 2)
#define idForeachTreeLast (idForeachChainElementLast + 2)


#define idGetChainElementTreeThis 0
#define idGetChainElementTreeLast 0
class tree : public chainElement, public chain
{	public:
	// Do not overload this function! Use toBeCalledPre() and toBeCalledPost()
	virtual int toBeCalledForeachElement(unsigned int iMsg, void *pDummy);
	virtual int toBeCalledPre(unsigned int iMsg, void *pDummy);
	virtual int toBeCalledPost(unsigned int iMsg, void *pDummy);
	tree(chain *pParent);
	virtual ~tree(void);
	// Calls bubbleSort() for every member
	virtual void bubbleSortTree(unsigned int iMsg);
	virtual void *getChainElementMember(unsigned int iMsg)
	{	switch (iMsg)
		{	default:
				abort();
				return (void*)"Hallo Peter";
			case idGetChainElementTreeThis:
				return (void *)this;
		}
	}
	virtual void *getChainMember(unsigned int iMsg)
	{       switch (iMsg)
		{	default:
				return getChainElementMember(iMsg);
		}
	}
	tree *getParent(void)
	{	return (tree*)pParent->getChainMember(idGetChainElementTreeThis);
	}
	unsigned int getDepth(void)
	{	if (pParent)
			return getParent()->getDepth() + 1;
		else
			return 0;
	}
	tree *getRoot(void)
	{	if (pParent)
			return getParent()->getRoot();
		else
			return this;
	}
	Boolean hasSameRoot(tree *pTree)
	{	return getRoot() == pTree->getRoot();
	}
	tree *getHighestCommonRoot(tree *pTree)
	{	unsigned int iDepth0;
		unsigned int iDepth1;

		if (pTree == this)
			return this;
		if ((iDepth0 = getDepth()) > (iDepth1 = pTree->getDepth()))
			return getParent()->getHighestCommonRoot(pTree);
		else
			if (iDepth0 < iDepth1)
				return getHighestCommonRoot(pTree->getParent());
			else
				return getParent()->getHighestCommonRoot(pTree->getParent());
	}
};


#endif /* !TREE_H */
