// test.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <SQLite.h>
#include <iostream>
#include <string>


using namespace std;

int main(int argc, CHAR* argv[])
{
	try
	{
		SQLite::MemoryDB s;
		s.execute("CREATE TEMP TABLE test (id INTEGER PRIMARY KEY AUTOINCREMENT, value TEXT);");
		SQLite::Stmt stmt = s.prepare("INSERT INTO test VALUES(?, ?);");
		int i = 0;
		for(string str;;++i)
		{
			cout << "new: ";
			cin >> str;
			if (str == "quit" || str == "exit")
				break;
			cout << "val: [" << str << " (" << i << ")]" << endl;

			stmt.bind(1, i);
			stmt.bind(2, str);
			stmt.execute();
		}
		stmt = s.prepare("SELECT * FROM test;");
		cerr << stmt.getQuery() << endl;
		while (stmt.next())
		{
			int i = stmt[0];
			string t = stmt[1];
			cout << "i: " << i << " t: [" << t << "]" << endl;
		}
		
		class MyFunc : public SQLite::Function
		{
		public:
			MyFunc() : SQLite::Function("test", 1) {}
			void operator()(SQLite::Context &ctx)
			{
				cout << ctx[0].asString();
				cout << " ( == " << (ctx[0] == "err") << " )";
				cout << " ( != " << (ctx[0] != "err") << " )";
				cout << " ( < "  << (ctx[0] <  "err") << " )";
				cout << " ( >= " << (ctx[0] >= "err") << " )";
				cout << " ( > "  << (ctx[0] >  "err") << " )";
				cout << " ( <= " << (ctx[0] <= "err") << " )";
				cout << endl;
				if (ctx[0] == "err")
					ctx.error("fuck!");
				ctx.set((int)ctx[0] + 1);
			}
		};
		s.registerFunction(new MyFunc);
		stmt = s.prepare("SELECT id, test(value) FROM test;");
		while (stmt.next())
		{
			cout << "a: " << (int)stmt[0] << " b: " << (int)stmt[1] << endl;
		}
		
		cout << "inserting 10000" << endl;
		stmt = s.prepare("INSERT INTO test VALUES(NULL, ?)");
		s.begin();
		for (int i = 0; i < 10000; ++i)
		{
			stmt.bind(1, i);
			stmt.execute();
		}
		cout << "commit" << endl;
		s.commit();
		
		cout << "end" << endl;
		stmt = s.prepare("SELECT COUNT(*) FROM test");
		stmt.next();
		cout << stmt[0].asInt() << endl;

		stmt = s.prepare("SELECT value FROM test WHERE id <= 10");
		while (stmt.next())
		{
			cout << stmt[0].asString() << endl;
		}
	} catch (SQLite::Exception &e)
	{
		cerr << "Exception: " << e.getErrorMsg() << " (" << e.getErrorCode() << ")" << endl;
	}
	system("pause");
	return 0;
}