#ifndef CHAIN_H
#define CHAIN_H


#include "object.h"


// There is a foreach() method which calls toBeCalledForeachElement()
// for each element which is contained in an object of class chain.
// There will be passed a message and a pointer.
// If you make a derived class and you define some messages,
// you should use idForeachChainElementLast + 1 as the first id
// See toBeCalledForeachElement of derived classes.
// If the message passed to toBeCalledForeachElement()
// is not defined in this class, the base class method will be called.
#define idForeachChainElementDestruct 0
#define idForeachChainElementMove 1
#define idForeachChainElementLast 1


class chainElement;


class chain : virtual public mtxObject		// container class
{	public:
	chainElement *pFirst, *pLast;
	unsigned int iNumberOfElements;
	chain(void);
	virtual ~chain(void);
	int foreach(unsigned int iMsg, void *pDummy);	// calls for each chainElement
	// Makes a bubblesort of the elements
	virtual void bubbleSort(unsigned int iMsg);
	// classes derived fro class chainElement can access to
	// their parent by the member pParent.
	// Caused of that pParent is of type (chain*)
	// you cannot access to members of derived classes from chain
	// but you can use this function
	// See derived classes
	// DO NOT CAST OBJECTS!!!
	// If a message is not defined in a class the base class function will
	// be called
	virtual void *getChainMember(unsigned int iMsg)
	{	(void)iMsg;
		abort();
		return (void*)"DO NOT TRY THIS AT HOME!!!";
	}
};


class chainElement : virtual public mtxObject	// double linked list
{	public:
	chain *pParent;			// container
	chainElement *pPrev, *pSucc;
	chainElement(chain *pParentNew);
	chainElement(void);
	// See above comment for getChainMember()
	// Sometime you want to access a member of a derived class via
	// chain.pFirst or chainElement.pSucc
	// use this method
	virtual void *getChainElementMember(unsigned int iMsg)
	{	(void)iMsg;
		abort();
		return (void*)"DO NOT TRY THIS AT HOME!!!";
	}
	virtual ~chainElement(void);
	// Will be called from chain.foreach()
	virtual int toBeCalledForeachElement(unsigned int iMsg, void *pDummy);
	// inserts a chainElement() which is not bound to a chain into
	// the passed chain
	virtual void insertSorted(chain *pParentNew, unsigned int iMsg);
	// Used for sorting.
	// The message identifies kind of sorting.
	// Base class method should be called if the message is not defined
	virtual Boolean laterThenThis(chainElement *pElement,
	    unsigned int iMsg);
	// Exchanges this with pElement
	virtual void exchange(chainElement *pElement);
	// Moves this from one chain into the passed new one
	virtual void move(chain *pChain);
	// See mtxObject
	virtual void getMutualExclusiveAccess(void);
	virtual void freeMutualExclusiveAccess(void);
#ifdef DEBUG
	void checkConsistence(void);
#endif /* DEBUG */
};


#endif /* !CHAIN_H */
