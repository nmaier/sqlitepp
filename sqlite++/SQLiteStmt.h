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

#ifndef _SQLITEPP_STMT_H
#define _SQLITEPP_STMT_H

#ifdef MSVC
#pragma once
#endif

#include <string>
#include <sqlite3.h>

using namespace std;

namespace SQLite
{
	class DB;

	class Stmt
	{
		friend class DB;
		friend class Data;
		friend class Exception;
	private:
		sqlite3_stmt *stmt;

		string query;
		string tail;

		DB *owner;

		bool ok, done, result;

	private:
		Stmt(DB *aOwner, const string &aQuery);

	public:
		Stmt(const Stmt &c);
		~Stmt(void);
		
		Stmt& operator=(const Stmt &c);
		
		void prepare();
		void check();
		void reset();
		void finalize();
		void bind(unsigned idx);
		void bind(unsigned idx, int value);
		void bind(unsigned idx, __int64 value);
		void bind(unsigned idx, double value);
		void bind(unsigned idx, string value);
		void bind(unsigned idx, void *value, unsigned length);

		const string& getQuery() const { return query; }
		const string& getTail() const { return tail; }

		unsigned getRowCount();
		unsigned getColumnCount();

		bool next();
		void execute() { next(); reset();}
		void rewind() { reset(); }

		Data value(unsigned idx);
		Data operator[](unsigned idx) { return value(idx); }
	};
}
#endif