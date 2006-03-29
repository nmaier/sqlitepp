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
#ifndef _SQLITEPP_DATA_H
#define _SQLITEPP_DATA_H

#ifdef MSVC
#pragma once
#endif

#include <string>
#include <sqlite3.h>

using namespace std;

namespace SQLite
{
	class Blob
	{
	private:
		const void *data;
		const unsigned length;

	public:
		explicit Blob(const void *aData, const unsigned aLength)
			: data(aData), length(aLength)
		{}

		operator const void*() const { return data; }
		const unsigned getLength() const { return length; }
		const void *getData() const { return data; }

		bool operator ==(const Blob &c) const;
		bool operator !=(const Blob &c) const { return !operator==(c); }

		bool operator >(const Blob &c) const;
		bool operator <=(const Blob &c) const { return !operator>(c); }

		bool operator <(const Blob &c) const;
		bool operator >=(const Blob &c) const { return !operator<(c); }


	};

	class BaseData
	{
	protected:
		BaseData() {}

	protected:
		virtual int asInt() const = 0;
		virtual __int64 asInt64() const = 0;
		virtual double asDouble() const = 0;
		virtual const char* asChar() const = 0;
		virtual string asString() const = 0;
		virtual Blob asBlob() const = 0;
	
		virtual int getType() const = 0;

	public:
		operator int() { return asInt(); }
		operator __int64() { return asInt64(); }
		operator double() { return asDouble(); }
		operator const char*() { return asChar(); }
		operator string() { return asString(); }
		operator Blob() { return asBlob(); }
		
		bool operator ==(const int v) const { return v == asInt(); }
		bool operator ==(const __int64 v) const { return v == asInt64(); }
		bool operator ==(const double v) const { return v == asDouble(); }
		bool operator ==(const char *v) const { return strcmp(v, asString().c_str()) == 0; }
		bool operator ==(const string& v) const { return v == asString(); }
		bool operator ==(const Blob &v) const { return v == asBlob(); }

		bool operator !=(const int v) const { return v != asInt(); }
		bool operator !=(const __int64 v) const { return v != asInt64(); }
		bool operator !=(const double v) const { return v != asDouble(); }
		bool operator !=(const char *v) const { return strcmp(v, asString().c_str()) != 0; }
		bool operator !=(const string& v) const { return v != asString(); }
		bool operator !=(const Blob &v) const { return v != asBlob(); }

		bool operator <(const int v) const { return v < asInt(); }
		bool operator <(const __int64 v) const { return v < asInt64(); }
		bool operator <(const double v) const { return v < asDouble(); }
		bool operator <(const char *v) const { return strcmp(v, asString().c_str()) == -1; }
		bool operator <(const string& v) const { return v < asString(); }
		bool operator <(const Blob &v) const { return v < asBlob(); }

		bool operator >=(const int v) const { return !operator<(v); }
		bool operator >=(const __int64 v) const { return !operator<(v); }
		bool operator >=(const double v) const { return !operator<(v); }
		bool operator >=(const char *v) const { return !operator<(v); }
		bool operator >=(const string &v) const { return !operator<(v); }
		bool operator >=(const Blob &v) const { return !operator<(v); }

		bool operator >(const int v) const { return v > asInt(); }
		bool operator >(const __int64 v) const { return v > asInt64(); }
		bool operator >(const double v) const { return v > asDouble(); }
		bool operator >(const char *v) const { return strcmp(v, asString().c_str()) == 1; }
		bool operator >(const string& v) const { return v > asString(); }
		bool operator >(const Blob &v) const { return v > asBlob(); }

		bool operator <=(const int v) const { return !operator>(v); }
		bool operator <=(const __int64 v) const { return !operator>(v); }
		bool operator <=(const double v) const { return !operator>(v); }
		bool operator <=(const char *v) const { return !operator>(v); }
		bool operator <=(const string &v) const { return !operator>(v); }
		bool operator <=(const Blob &v) const { return !operator>(v); }

	};

	class Data : public BaseData
	{
		friend class Stmt;
	
	private:
		Stmt *stmt;
		unsigned idx;
	
	private:
		Data(Stmt *aStmt, unsigned aIdx);
	public:
		Data(const Data &c)
		{
			idx = c.idx;
			stmt = c.stmt;
		}
		virtual int asInt() const;
		virtual __int64 asInt64() const;
		virtual double asDouble() const;
		virtual const char *asChar() const;
		virtual string asString() const;
		virtual Blob asBlob() const;

		virtual int getType() const;
	};

	class Value : public BaseData
	{
		sqlite3_value* val;
	public:
		Value(sqlite3_value *aVal)
			: val(aVal)
		{}
		virtual int asInt() const { return sqlite3_value_int(val); }
		virtual __int64 asInt64() const { return sqlite3_value_int64(val); }
		virtual double asDouble() const { return sqlite3_value_double(val); }
		virtual const char* asChar() const { return (const char*)sqlite3_value_text(val); }
		virtual string asString() const { return string((const char*)sqlite3_value_text(val)); }
		virtual Blob asBlob() const;

		virtual int getType() const { return sqlite3_value_type(val); }
	};
}
#endif