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
#ifndef _SQLITEPP_EXCEPT_H
#define _SQLITEPP_EXCEPT_H

#ifdef MSVC
#pragma once
#endif

#include <string>
#include <sqlite3.h>

namespace SQLite
{
	class Exception
	{
		friend class SQLite;

		string error;
		int errorCode;

	public:

		Exception(sqlite3 *ctx)
		{
			error.assign(sqlite3_errmsg(ctx));
			errorCode = sqlite3_errcode(ctx);
		}
		Exception(const string &aMsg)
			: error(aMsg), errorCode(-1)
		{}
		Exception(const int aCode)
			: errorCode(aCode)
		{
			if (errorCode == SQLITE_OK)
			{
				error.assign("No Error");
			}
			else if (errorCode == SQLITE_BUSY)
			{
				error.assign("Database is busy");
			}
			else if (errorCode == SQLITE_NOMEM)
			{
				error.assign("No memory");
			}
			else if (errorCode == SQLITE_MISUSE)
			{
				error.assign("Misuse");
			}
			else
			{
				error.assign("General error");
			}
		}

		~Exception(void)
		{
		}

		const string &getErrorMsg() const
		{
			return error;
		}
		const int getErrorCode() const
		{
			return errorCode;
		}
	};
}
#endif