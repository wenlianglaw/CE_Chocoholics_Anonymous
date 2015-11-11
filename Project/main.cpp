#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <string.h>
#include <conio.h>
#include <string>

#include "mysql_driver.h"
#include "mysql_connection.h"
#include "cppconn/driver.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/metadata.h"
#include "cppconn/exception.h"


using namespace sql;
using namespace std;

#define CATCHSQL catch (SQLException &e) {\
				cout << "# ERR: SQLException in " << __FILE__;\
				cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;\
				cout << "# ERR: " << e.what();\
				cout << " (MySQL error code: " << e.getErrorCode();\
				cout << ", SQLState: " << e.getSQLState() << " )" << endl;\
				}

enum Operater {PROVIDER = 0x100, MEMBER=0x01,MANAGER=0x10 };


class MyConnector {
public:
	MyConnector() {
		try {
			/* Create a connection */
			driver = get_driver_instance();
			con = driver->connect("tcp://127.0.0.1:3306", "root", "123456");
			/* Connect to the MySQL test database */
			con->setSchema("chocoholics");
			stmt = con->createStatement();
		}
		catch (SQLException &e) {
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		}
	}
	~MyConnector() {
		try {
			delete res;
			delete stmt;
			delete con;
		}catch (SQLException &e) {
			cout << "# ERR: SQLException in " << __FILE__;
			cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "# ERR: " << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << ", SQLState: " << e.getSQLState() << " )" << endl;
		}
	}
	sql::Driver *driver;
	sql::Connection *con;
	sql::Statement *stmt;
	sql::ResultSet *res;
};


class Member {
private:
	string name;
	int memberID;
public:
	string getName() { return name; }
	int getMemberID() { return memberID; }

	Member(string name, int number) {
		this->name = name;
		memberID = number;
	}
	Member() :name(string()), memberID(0) {}
};

class Provider {
private:
	string name;
	int providerId;
public:
	string getName() { return name; }
	int getProviderID() { return providerId; }
};

class LoginMessage {
	
};

class Login {
public:
	int validateUsername(const string &username)const {
		for (auto ch : username)
			if (!(ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'z' ||
				ch >= 'A' && ch <= 'Z')) {
				return 0;
			}
		return 1;
	}
	int validatePassword(const string &pwd){
		return 1;
	}

	// TODO:: MD5 or other algorithm
	string encryptPWD(const string &pwd)const {
		return pwd;
	}
	/*
		success return OPERATOR
		fail return 0;
	*/
	int login() {
		string username ="";
		string password ="";
		printf("UserName:");
		cin >> username;
		if (!validateUsername(username)) {
			cout << "Not validated username" << endl;
			return 0;
		}
		char ch = _getch();
		while (ch != 13) {
			password.push_back(ch);
			cout << "*";
			ch = _getch();
		}
		if (!validatePassword(password)) {
			cout << "Not validated password" << endl;
			return 0;
		}
		cout << endl;
		password = encryptPWD(password);
		try {
			/* fetch the username and pwd in database */
			MyConnector MC;
			MC.res = MC.stmt->executeQuery("select * from provider where Name = '" + username + "'");
			if (MC.res->next()) {
				string dbPWD = MC.res->getString("pwd");
				if (dbPWD == password) {
					//TODO return type according result.type
					printf("Login successful! Your Identity is :%s", "provider");
					return PROVIDER;
				}
			} else {
				return 0;
			}
		}CATCHSQL;
		return 0;
	}
};

class Verify{
public:
	int verify(LoginMessage msg) {
		return 1;
	}
	
};

class Service {
private:
	string serviceName;
	int serviceCode;
	int m_root;

public:
	bool operator <(const Service &l)const {
		return (l.serviceCode < serviceCode);
	}
public:
	string name() {
		return serviceName;
	}
	int code() {
		return serviceCode;
	}
	int root() { return m_root; }

	void print() {
		printf("Service Name:%s\n"
			"Service Code:%d\n", serviceName.c_str(), serviceCode);
	}

public:
	Service(string Name, int Code, int root = 0) {
		serviceName = Name;
		serviceCode = Code;
		m_root = root;
	}
	Service() :serviceCode(0), serviceName(string()), m_root(0) {}
};


set<Service> g_serviceDirectory;

void createService() {
	g_serviceDirectory.insert(Service("ÒûÊ³¿ØÖÆ", 1001, 1000));
}


class ServiceCenter {
public:
	void getAllServices() {
		for (auto i : g_serviceDirectory) {
			i.print();
		}
	}
	Service getServiceByCode() {

	}
};

class CASystem {
private:
	Operater curOperator;
public:
	Operater getCurOperator() { return curOperator; }
	ServiceCenter SC;
	vector<Member> members;
	vector<Provider> providers;
	void showInstructions() {
		int i=0;
		printf("\n\n");
		// 0
		printf("[%d]:Save & Exit\n", i); i++;
		printf("[%d]:Show All Services\n", i); i++;
		printf("[%d]:Generate Bills\n", i); i++;
		printf("[%d]:Save & Exit\n", i); i++;
		printf("[%d]:Save & Exit\n", i); i++;
		printf("\n\n\n\n");
		// 5
	}
	
public:
	void Run() {
		Login L;
		int cmd = 0;
		int loginFailtimes = 0;
		while (!L.login()) {
			cout << "wrong login information, Try again" << endl;
			loginFailtimes++;
			if (loginFailtimes >= 5) {
				cout << "EXIT.." << endl;
				cmd = -1;
			}
		}

		if (cmd == -1) {
			//exit
			return;
		}
		while (1) {
			showInstructions();
			scanf("%d", &cmd);
			switch (cmd) {
			case 0:
				break;
			case 1:
				printf("Show All Services\n");
				SC.getAllServices(); break;
			case 2:break;
			case 3:break;
			default:break;
			}
		}		
	}
	void readFile() {
		FILE *fp;
		fp = fopen("Member.txt", "r");

		if (!fp) {
			printf("cannot find the file\nexit\n");
			exit(-1);
		}
		char line[256];
		while (fgets(line, sizeof(line), fp) != NULL) {
			int mNumber = atoi(strtok(line, ",")); 
			char *mName = strtok(NULL,"");
			members.push_back(Member(mName, mNumber));
		}
		fclose(fp);
	}
	int grant(int compairedLvl) {
		if (compairedLvl & curOperator) return 1;
		printf("You are not allowed to opearte this function\n");
		return 0;
	}
protected:
};
void initial_test() {
	createService();
}





void myConnectionSQL() {
	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "123456");
		/* Connect to the MySQL test database */
		con->setSchema("chocoholics");
		stmt = con->createStatement();
		res = stmt->executeQuery("select * from provider");
		while (res->next()) {

			// You can use either numeric offsets...
			cout << "id = " << res->getInt(1); // getInt(1) returns the first column
											   // ... or column names for accessing results.
											   // The latter is recommended.
			cout << "password = '" << res->getString("pwd") << "'" << endl;
		}

		delete res;
		delete stmt;
		delete con;
	}
	catch (SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line "<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}
}

int main() {
	CASystem CC;

	initial_test();
	
	CC.Run();

	return EXIT_SUCCESS;
}


