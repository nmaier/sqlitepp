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
/* $Id$ */

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
        explicit Exception(const std::string& error)
            : ::Exception(error.c_str())
        {
        }
        explicit Exception(const AnsiString& error)
            : errorCode(-1), Sysutils::Exception(error)
        {
        }
        explicit Exception(const char *error)
            : errorCode(-1), Sysutils::Exception(error)
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
        std::string error;

    public:

        Exception(sqlite3 *ctx)
        {
            error.assign(sqlite3_errmsg(ctx));
            errorCode = sqlite3_errcode(ctx);
        }
        Exception(const std::string &aMsg)
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

        const std::string &getErrorMsg() const
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
        void set(const int v) { sqlite3_result_int(ctx, v); }
        void set(const __int64& v) { sqlite3_result_int64(ctx, v); }
        void set(const double& v) { sqlite3_result_double(ctx, v); }
        void set(const std::string& v)
        {
            sqlite3_result_text(ctx, v.c_str(), (int)v.length(), SQLITE_TRANSIENT);
        }
        void set(const void *data, const unsigned length)
        {
            sqlite3_result_blob(ctx, data, (int)length, SQLITE_TRANSIENT);
        }
        Value operator[](int idx);
    };

    class Function
    {
        friend class DB;
    private:
        std::string name;
        int args;

        std::string error;

    public:
        Function(const std::string& aName, int aArgs);

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

        operator const void *() const { return data; }
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
        virtual std::string asString() const = 0;
        virtual Blob asBlob() const = 0;
#ifdef __BORLANDC__
        virtual AnsiString asAString() const = 0;
#endif
        virtual int getType() const = 0;

    public:
        operator int() const { return asInt(); }
        operator __int64() const { return asInt64(); }
        operator double() const { return asDouble(); }
        operator const char *() const { return asChar(); }
        operator std::string() const { return asString(); }
#ifdef __BORLANDC__
        operator AnsiString() const;
#endif
        operator Blob() const { return asBlob(); }

        bool operator ==(const int v) const { return v == asInt(); }
        bool operator ==(const __int64 v) const { return v == asInt64(); }
        bool operator ==(const double v) const { return v == asDouble(); }
        bool operator ==(const char *v) const { return strcmp(v, asString().c_str()) == 0; }
        bool operator ==(const std::string& v) const { return v == asString(); }
#ifdef __BORLANDC__
        bool operator ==(const AnsiString& v) const { return v == asAString(); }
#endif
        bool operator ==(const Blob &v) const { return v == asBlob(); }

        bool operator !=(const int v) const { return v != asInt(); }
        bool operator !=(const __int64 v) const { return v != asInt64(); }
        bool operator !=(const double v) const { return v != asDouble(); }
        bool operator !=(const char *v) const { return strcmp(v, asString().c_str()) != 0; }
        bool operator !=(const std::string& v) const { return v != asString(); }
#ifdef __BORLANDC__
        bool operator !=(const AnsiString& v) const { return v != asAString(); }
#endif
        bool operator !=(const Blob &v) const { return v != asBlob(); }

        bool operator <(const int v) const { return v < asInt(); }
        bool operator <(const __int64 v) const { return v < asInt64(); }
        bool operator <(const double v) const { return v < asDouble(); }
        bool operator <(const char *v) const { return strcmp(v, asString().c_str()) == -1; }
        bool operator <(const std::string& v) const { return v < asString(); }
#ifdef __BORLANDC__
        bool operator <(const AnsiString& v) const { return v < asAString(); }
#endif
        bool operator <(const Blob &v) const { return v < asBlob(); }

        bool operator >=(const int v) const { return !operator<(v); }
        bool operator >=(const __int64 v) const { return !operator<(v); }
        bool operator >=(const double v) const { return !operator<(v); }
        bool operator >=(const char *v) const { return !operator<(v); }
        bool operator >=(const std::string &v) const { return !operator<(v); }
#ifdef __BORLANDC__
        bool operator >=(const AnsiString &v) const { return !operator<(v); }
#endif
        bool operator >=(const Blob &v) const { return !operator<(v); }

        bool operator >(const int v) const { return v > asInt(); }
        bool operator >(const __int64 v) const { return v > asInt64(); }
        bool operator >(const double v) const { return v > asDouble(); }
        bool operator >(const char *v) const { return strcmp(v, asString().c_str()) == 1; }
        bool operator >(const std::string& v) const { return v > asString(); }
#ifdef __BORLANDC__
        bool operator >(const AnsiString& v) const { return v > asAString(); }
#endif
        bool operator >(const Blob &v) const { return v > asBlob(); }

        bool operator <=(const int v) const { return !operator>(v); }
        bool operator <=(const __int64 v) const { return !operator>(v); }
        bool operator <=(const double v) const { return !operator>(v); }
        bool operator <=(const char *v) const { return !operator>(v); }
        bool operator <=(const std::string &v) const { return !operator>(v); }
#ifdef __BORLANDC__
        bool operator <=(const AnsiString &v) const { return !operator>(v); }
#endif
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
        virtual std::string asString() const;
#ifdef __BORLANDC__
        virtual AnsiString asAString() const;
#endif
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
        virtual std::string asString() const;
#ifdef __BORLANDC__
        virtual AnsiString asAString() const;
#endif
        virtual Blob asBlob() const;

        virtual int getType() const { return sqlite3_value_type(val); }
    };

    class DB;
    class Stmt;

    class DataItr
    {
    public:
        virtual bool next() = 0;
        virtual void bind(Stmt& aStmt) = 0;
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


    class Stmt
    {
        friend class DB;
        friend class Data;
        friend class Exception;
    private:
        sqlite3_stmt *stmt;

        std::string query;
        std::string tail;

        DB& owner;

        bool ok, done, result;

    private:
        Stmt(DB& aOwner, const std::string &aQuery);
#ifdef __BORLANDC__
        Stmt(DB& aOwner, const AnsiString &aQuery);
#endif
    public:
        Stmt(const Stmt &c);
        ~Stmt(void);

        Stmt& operator=(const Stmt &c);

        void prepare();
        void check();
        void reset();
        void finalize();

        void bind(unsigned idx);
        void bind(unsigned idx, const int value);
        void bind(unsigned idx, const __int64& value);
        void bind(unsigned idx, const double& value);
        void bind(unsigned idx, const std::string& value);
#ifdef __BORLANDC__
        void bind(unsigned idx, const AnsiString& value);
#endif
        void bind(unsigned idx, const void *value, const unsigned length);

        const std::string& getQuery() const { return query; }
        const std::string& getTail() const { return tail; }

        unsigned getRowCount();
        unsigned getColumnCount();

        bool next();
        void execute() { next(); reset();}
        void executeMany(DataItr& dataProvider, Trans::TransactionType aType = Trans::DEFERRED);
        void rewind() { reset(); }

        Data value(unsigned idx);
        Data operator[](unsigned idx) { return value(idx); }
    };

    class Finalizer
    {
    private:
        Stmt& stmt;
    public:
        Finalizer(Stmt& aStmt);
        ~Finalizer();
    };

    class Resetter
    {
    private:
        Stmt& stmt;
    public:
        Resetter(Stmt& aStmt);
        ~Resetter();
    };

    std::string escape(const std::string &in);
    std::string _cdecl mprintf(const char *, ...);

    class DB
    {
        friend class Stmt;

    private:
        sqlite3 *ctx;
        std::string db;

        typedef std::deque<Function*> FuncList;
        FuncList funcs;

        void open(const char *aDB);

    public:
        explicit DB(const char *aDB);
        explicit DB(const std::string& aDB);
#ifdef __BORLANDC__
        explicit DB(const AnsiString& aDB);
#endif
        virtual ~DB();

        Stmt prepare(const std::string &aQuery);
        Stmt prepare(const char *aQuery);        
        void execute(const std::string &aQuery);
        void executeMany(const std::string &aQuery, DataItr& dataProvider, Trans::TransactionType aType = Trans::DEFERRED);

#ifdef __BORLANDC__
        Stmt prepare(const AnsiString &aQuery);
        void execute(const AnsiString &aQuery);
        void executeMany(const AnsiString &aQuery, DataItr& dataProvider, Trans::TransactionType aType = Trans::DEFERRED);

#endif

        void __cdecl execute(const char *aQuery, ...);

        const std::string& getDB() const { return db; }

        void registerFunction(Function *aFunc);

        __int64 lastInsertId() const ;

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