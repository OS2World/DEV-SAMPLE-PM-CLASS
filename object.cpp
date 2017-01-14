#include "object.h"
extern "C"
{
#include <memory.h>
};


object::object(void)
{	bSuccessfullConstructed = TRUE;
}


object::~object(void)
{
}


/*
Remembers the size of the object to memset it to zero when deleting.
This helps finding bugs caused of destructing objects more then once.
*/
void *object::operator new(unsigned int iSize)
{	unsigned int *pObject;

	if (!(pObject = (unsigned int*)::new char[iSize + 8]))
		return (void *)0;
	*pObject = iSize;
	memset((char *)pObject += 8, 0, iSize);
	return (void*)pObject;
}


void object::operator delete(void *pObject)
{	unsigned char *pSize;

	memset(pObject, 0, *(pSize = ((unsigned char*)pObject - 8)));
	::delete pSize;
}


void construct::doSomething(void)
{
}


//     Call it direct only if init() is called direct
//     if run() is not used caused of that no dosSomething exists
//     - e.g. windows
void construct::destructAll()
{	if (isSuccessfull())
	{	destructPre();
		destruct();
		destructPost();
	}
}


void construct::run(void)
{	if (init())
	{	doSomething();
		destructAll();
	}
}


// You can see that there is a matching counterpart for every init function.
//	initPre() - destructPost()
//	create() - destruct()
//	initPost() - destructPre()
Boolean construct::init(void)
{	if (isSuccessfull())
	{	if (initPre())
			if (create())
			{	if (initPost())
					return TRUE;
				else
				{	destruct();
					destructPost();
					return FALSE;
				}
			}
			else
			{	destructPost();
				return FALSE;
			}
		else
			return FALSE;
	}
	else
		return FALSE;
}


Boolean construct::initPre(void)
{	return TRUE;
}


Boolean construct::create(void)
{	return TRUE;
}


Boolean construct::initPost(void)
{	return TRUE;
}


void construct::destructPost(void)
{
}


void construct::destructPre(void)
{
}


void construct::destruct(void)
{
}
