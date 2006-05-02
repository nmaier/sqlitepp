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
#ifndef _SQLITEPP_FUNC_H
#define _SQLITEPP_FUNC_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include <string>
#include <sqlite3.h>

using namespace std;

namespace SQLite
{
	class Value;
	class Context
	{
		sqlite3_context *ctx;
		int args;
		sqlite3_value **vals;

	public:
		Context(sqlite3_context *aCtx, int aArgs, sqlite3_value **aVals)
			: ctx(aCtx), args(aArgs), vals(aVals)
		{}
		void error(const char *error) { sqlite3_result_error(ctx, error, (int)strlen(error)); }
		void set() { sqlite3_result_null(ctx); }
		void set(int v) { sqlite3_result_int(ctx, v); }
		void set(__int64 v) { sqlite3_result_int64(ctx, v); }
		void set(double v) { sqlite3_result_double(ctx, v); }
		void set(string v)
		{
			char *t = (char*)malloc(v.length() + 1);
#if _MSC_VER >= 1400
			strcpy_s(t, v.length() + 1, v.c_str());
#else
			strcpy(t, v.c_str());
#endif
			sqlite3_result_text(ctx, t, (int)v.length(), free);
		}
		void set(void *data, unsigned length)
		{
			void *p = malloc(length);
			memcpy(p, data, length);
			sqlite3_result_blob(ctx, data, (int)length, free);
		}
		Value operator[](int idx);
	};

	class Function
	{
		friend class DB;
	private:
		string name;
		int args;

		string error;
		
	public:
		Function(string aName, int aArgs);
		
		virtual void operator()(Context &) = 0;
	};
}
#endif