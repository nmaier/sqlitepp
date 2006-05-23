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

#ifdef __BORLANDC__
#   include <Classes.hpp>
#endif

using namespace std;

namespace SQLite
{
#ifdef __BORLANDC__
    class Exception : public ::Exception
    {
    public:
        explicit Exception(sqlite3 *ctx)
            : ::Exception("")
        {
            AnsiString m;
            m.sprintf(
                "%s (Code: %d)",
                sqlite3_errmsg(ctx),
                sqlite3_errcode(ctx)
            );
            Message = m;
            errorCode = sqlite3_errcode(ctx);
        }
        explicit Exception(const string& error)
            : ::Exception(error.c_str())
        {
        }
        explicit Exception(const AnsiString& error)
            : errorCode(-1), ::Exception(error)
        {
        }
        explicit Exception(const char *error)
            : errorCode(-1), ::Exception(error)
        {
        }

        explicit Exception(const int aCode)
            : errorCode(aCode), ::Exception("")
        {
            if (errorCode == SQLITE_OK)
            {
                Message = "No Error";
            }
            else if (errorCode == SQLITE_BUSY)
            {
                Message = "Database is busy";
            }
            else if (errorCode == SQLITE_NOMEM)
            {
                Message = "No memory";
            }
            else if (errorCode == SQLITE_MISUSE)
            {
                Message = "Misuse";
            }
            else
            {
                Message = "General error";
            }
        }

#else
    class Exception
    {
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

        const string &getErrorMsg() const
        {
            return error;
        }
#endif
    private:
        int errorCode;

    public:
        const int getErrorCode() const
        {
            return errorCode;
        }
    };

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
        void set(const string& v)
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

    class DB;
    class Stmt;
    class DataItr
    {
    public:
        virtual bool next() = 0;
        virtual bool bind(Stmt& aStmt) = 0;
    };
    class Stmt
    {
        friend class DB;
        friend class Data;
        friend class Exception;
    private:
        sqlite3_stmt *stmt;

        string query;
        string tail;

        DB& owner;

        bool ok, done, result;

    private:
        Stmt(DB& aOwner, const string &aQuery);

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
        void bind(unsigned idx, const string& value);
#ifdef __BORLANDC__
        void bind(unsigned idx, const AnsiString& value);
#endif
        void bind(unsigned idx, void *value, unsigned length);

        const string& getQuery() const { return query; }
        const string& getTail() const { return tail; }

        unsigned getRowCount();
        unsigned getColumnCount();

        bool next();
        void execute() { next(); reset();}
        void executeMany(DataItr& dataProvider);
        void rewind() { reset(); }

        Data value(unsigned idx);
        Data operator[](unsigned idx) { return value(idx); }
    };

    class Trans
    {
    public:
        enum TransactionType {DEFERRED, IMMEDIATE, EXCLUSIVE};

    private:
        bool finished;
        DB& db;
        void finish();

    public:
        Trans(DB& aDB, TransactionType aType = DEFERRED);
        ~Trans();
        void commit();
        void rollback();
    };

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

        void open(const char *aDB);

    public:
        explicit DB(const char *aDB);
        explicit DB(const string& aDB);
#ifdef __BORLANDC__
        explicit DB(const AnsiString& aDB);
#endif
        virtual ~DB();

        Stmt prepare(const string &aQuery);
        void execute(const string &aQuery);
        void __cdecl execute(const char *aQuery, ...);

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