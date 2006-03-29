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
#ifndef _SQLITEPP_H
#define _SQLITEPP_H

#ifdef MSVC
#pragma once
#endif

#include <string>
#include <deque>
#include <sqlite3.h>

using namespace std;

#include "SQLiteException.h"
#include "SQLiteFunction.h"
#include "SQLiteData.h"
#include "SQLiteStmt.h"

namespace SQLite
{
	enum TransactionType {DEFERRED, IMMEDIATE, EXCLUSIVE};	

	string escape(const string &in);
	string _cdecl mprintf(const char *, ...);

	class DB
	{
		friend class Stmt;

	private:
		sqlite3 *ctx;
		string db;

		bool inTrans;

		typedef deque<Function*> FuncList;
		FuncList funcs;

	public:
		DB(const string &aDB);
		virtual ~DB();

		Stmt prepare(const string &aQuery);
		void execute(const string &aQuery);

		void begin(TransactionType aType = DEFERRED);
		void commit();
		void rollback();

		const string& getDB() const { return db; }

		const bool inTransaction() const { return inTrans; }

		void registerFunction(Function *aFunc);
	};
	class MemoryDB : public DB
	{
	public:
		MemoryDB(): DB(":memory:") {}
	};
	class TempDB : public DB
	{
	public:
		TempDB() : DB("") {}
	};
}
#endif