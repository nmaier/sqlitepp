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
	bool Blob::operator ==(const Blob &c) const
	{
		return length == c.length && (memcmp(data, c.data, length) == 0);
	}
	bool Blob::operator >(const Blob &c) const
	{
		unsigned u = min(length, c.length);
		int cmp;
		if (u && (cmp = memcmp(data, c.data, u)) != 0)
		{
			return (cmp == -1);
		}
		return length > c.length;
	}
	bool Blob::operator <(const Blob &c) const
	{
		unsigned u = min(length, c.length);
		int cmp;
		if (u && (cmp = memcmp(data, c.data, u)) != 0)
		{
			return (cmp == 1);
		}
		return length < c.length;
	}

	Data::Data(Stmt *aStmt, unsigned aIdx)
	: stmt(aStmt), idx(aIdx)
	{
#ifndef NDEBUG
		if (idx >= stmt->getColumnCount())
		{
			throw Exception("Invalid Column");
		}
#endif
	}
	int Data::asInt() const
	{
		return sqlite3_column_int(stmt->stmt, (int)idx);
	}
	__int64 Data::asInt64() const
	{
		return sqlite3_column_int64(stmt->stmt, (int)idx);
	}
	double Data::asDouble() const
	{
		return sqlite3_column_double(stmt->stmt, (int)idx);
	}
	const char *Data::asChar() const
	{
		return (const char*)sqlite3_column_text(stmt->stmt, (int)idx);
	}
	string Data::asString() const
	{
		string rv;
		const unsigned char *buf = sqlite3_column_text(stmt->stmt, (int)idx);
		if (buf)
		{
			rv.assign((const char*)buf);
		}
		return rv;
	}
#ifdef __BORLANDC__
	AnsiString Data::asAString() const
	{
		AnsiString rv;
		const unsigned char *buf = sqlite3_column_text(stmt->stmt, (int)idx);
		if (buf)
		{
			rv = AnsiString((const char*)buf);
		}
		return rv;
	}
#endif
	Blob Data::asBlob() const
	{
		return Blob(
				sqlite3_column_blob(stmt->stmt, idx),
				(const unsigned)sqlite3_column_bytes(stmt->stmt, idx)
				);
	}
	int Data::getType() const
	{
		return sqlite3_column_type(stmt->stmt, idx);
	}

	Blob Value::asBlob() const
	{
		return Blob(
			sqlite3_value_blob(val),
			sqlite3_value_bytes(val)
			);
	}
}