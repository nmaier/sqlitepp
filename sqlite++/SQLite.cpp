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

#include "sqlite.h"
#ifdef OS_WIN
#undef OS_WIN
#endif

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#define OS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif

static int busy_handler(void *, int attempts)
{
	if (attempts > 10)
	{
		return 0;
	}
#	if defined(OS_WIN)
	Sleep(attempts * 50);
#	elif defined(HAVE_USLEEP) && HAVE_USLEEP
	usleep(ms*1000);
#	else
	sleep(((attempts * 50) + 999) / 1000);
#	endif
	
	return 1;
}

static void func_handler(sqlite3_context *ctx, int args, sqlite3_value **vals)
{
	(*reinterpret_cast<SQLite::Function*>(sqlite3_user_data(ctx)))(SQLite::Context(ctx, args, vals));
}

namespace SQLite
{
	string escape(const string &in)
	{
		return mprintf("%q", in.c_str());
	}
	string _cdecl mprintf(const char *fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		char *o = sqlite3_vmprintf(fmt, ap);
		va_end(ap);
		
		string rv(o);
		sqlite3_free(o);
		return rv;
        
	}

	DB::DB(const string &aDB)
	: db(aDB), ctx(NULL), inTrans(false)
	{
		if (SQLITE_OK != sqlite3_open(db.c_str(), &ctx))
		{
			throw Exception(ctx);
		}
		sqlite3_busy_handler(
				ctx,
				busy_handler,
				NULL
				);
	}

	DB::~DB()
	{
		if (ctx)
		{
			sqlite3_close(ctx);
			ctx = NULL;
		}
		for (FuncList::iterator i = funcs.begin(), e = funcs.end(); i != e; ++i)
		{
			delete *i;
		}
	}

	Stmt DB::prepare(const string &aQuery)
	{
		return Stmt(this, aQuery);
	}
	void DB::execute(const string &aQuery)
	{
		prepare(aQuery).execute();
	}

	void DB::begin(TransactionType aType)
	{
		if (inTrans)
		{
			throw Exception("Already in Transaction");
		}
		string Query("BEGIN ");
		switch (aType)
		{
		case IMMEDIATE:
			Query.append("IMMEDIATE");
			break;
		case EXCLUSIVE:
			Query.append("EXCLUSIVE");
			break;
		default:
			Query.append("IMMEDIATE");
			break;
		}
		execute(Query);
		inTrans = true;
	}
	void DB::commit()
	{
		if (!inTrans)
		{
			throw Exception("Not in Transaction");
		}
		execute("COMMIT");
		inTrans = false;
	}
	void DB::rollback()
	{
		if (!inTrans)
		{
			throw Exception("Not in Transaction");
		}
		execute("ROLLBACK");
		inTrans = false;
	}

	void DB::registerFunction(Function *aFunc)
	{
		if (SQLITE_OK != sqlite3_create_function(
			ctx, aFunc->name.c_str(),
			aFunc->args,
			SQLITE_UTF8, aFunc,
			func_handler,
			NULL,
			NULL
		))
		{
			delete aFunc;
			throw Exception(ctx);
		}
		funcs.push_back(aFunc);
	}
}