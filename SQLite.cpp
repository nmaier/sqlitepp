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

#ifdef OS_WIN
#   undef OS_WIN
#endif

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
#   define OS_WIN
#   include <windows.h>
#else
#   include <unistd.h>
#endif

static int busy_handler(void *, int attempts)
{
	if (attempts < 1000)
	{
		// nasty. makes os scheduler context-switch
		Sleep(0);
		return 1;
	}
	else if (attempts < 1200)
	{
		Sleep(100);
		return 1;
	}

	return 0;
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
	Trans::Trans(DB& aDB, TransactionType aType)
		: db(aDB), finished(false)
	{
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
		db.execute(Query);
	}
	Trans::~Trans()
	{
		if (!finished)
		{
			rollback();
		}
	}
	void Trans::commit()
	{
		if (finished)
		{
			throw Exception("Transaction alreay finished");
		}
		db.execute("COMMIT");
		finished = true;
	}
	void Trans::rollback()
	{
		if (finished)
		{
			throw Exception("Transaction alreay finished");
		}
		db.execute("ROLLBACK");
		finished = true;
	}

	DB::DB(const char *aDB)
	: ctx(NULL)
	{
		open(aDB);
	}
	DB::DB(const string& aDB)
	: ctx(NULL)
	{
		open(aDB.c_str());
	}
#ifdef __BORLANDC__
	DB::DB(const AnsiString& aDB)
	: ctx(NULL)
	{
		open(aDB.c_str());
	}
#endif
	void DB::open(const char *aDB)
	{
		db = aDB;
		if (SQLITE_OK != sqlite3_open(aDB, &ctx))
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
		return Stmt(*this, aQuery);
	}
	Stmt DB::prepare(const char *aQuery)
	{
		return Stmt(*this, string(aQuery));
	}
	void DB::execute(const string &aQuery)
	{
		prepare(aQuery).execute();
	}
	void DB::executeMany(const string &aQuery, DataItr& dataProvider, Trans::TransactionType aType)
	{
		prepare(aQuery).executeMany(dataProvider, aType);
	}

#ifdef __BORLANDC__
	Stmt DB::prepare(const AnsiString &aQuery)
	{
		return Stmt(*this, aQuery);
	}
	void DB::execute(const AnsiString &aQuery)
	{
		prepare(aQuery).execute();
	}
	void DB::executeMany(const AnsiString &aQuery, DataItr& dataProvider, Trans::TransactionType aType)
	{
		prepare(aQuery).executeMany(dataProvider, aType);
	}
#endif

	void __cdecl DB::execute(const char* aQuery, ...)
	{
		va_list ap;
		va_start(ap, aQuery);
		char *o = sqlite3_vmprintf(aQuery, ap);
		va_end(ap);

		string Query(o);
		sqlite3_free(o);
		execute(Query);
	}


	__int64 DB::lastInsertId() const
	{
		return sqlite3_last_insert_rowid(ctx);
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
