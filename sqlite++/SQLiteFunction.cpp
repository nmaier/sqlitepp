/*
** 2006 March 18
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
*/
#include "SQLite.h"
using namespace std;

namespace SQLite
{
	Value Context::operator [](int idx)
	{
#ifndef NDEBUG
		if (idx >= args)
		{
			throw Exception(mprintf("Index out of Range(%d/%d)", idx, args));
		}
#endif
		return Value(vals[idx]);
	}

	Function::Function(const string& aName, int aArgs)
	: name(aName), args(aArgs)
	{}
}