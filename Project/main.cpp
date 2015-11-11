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
				system("pause");\
				}

enum Operator { NIL = 0x00,PROVIDER = 0x100, MEMBER=0x01,MANAGER=0x10 };
enum MemberStatus { FAIL = 0x00 ,VALIDATED, MEMBER_SUSPENDED, SUSPENDED };


/* Auxiliary Functions */
Operator stringToOperator(const string &str) {
	if (str == "Provider") 
		return PROVIDER;
	if (str == "Manager")
		return MANAGER;
	if (str == "Member")
		return MEMBER;
	return NIL;	
}

string operatorToString(const Operator& op) {
	switch (op) {
	case MANAGER:return string("Manager");
	case PROVIDER:return string("Provider");
	case MEMBER:return string("Member");
	case NIL: 
	default:return string("NULL");
	}
}

/************************/
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
		success return operator id
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
			//Backward
			if (ch == 8) {
				password.pop_back();
				printf("%c %c", 8,8);
			} else {
				password.push_back(ch);
				cout << "*";
			}
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
					int id = atoi(MC.res->getString("id").c_str());
					return id;
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
	MemberStatus stringToMS(const string &str)const {
		if (str == "VALIDATED")
			return VALIDATED;
		if (str == "MEMBER_SUSPENDED")
			return MEMBER_SUSPENDED;
		if (str == "SUSPENDED")
			return SUSPENDED;
		return MemberStatus::FAIL;
	}
	int checkFormat(const string &strNumber)const {
		return 1;
	}
	MemberStatus verify(const string& strNumber)const {
		if (!checkFormat(strNumber)) {
			return FAIL;
		}
		int memberNumber = stoi(strNumber);
		try {
			MyConnector MC;
			// Look in the DB, get the status
			MC.res = MC.stmt->executeQuery("select status from member where id = " + to_string(memberNumber));
			if (MC.res->next()) {
				MemberStatus MS = stringToMS(MC.res->getString(1));
				return MS;
			}
		}CATCHSQL;
	}
	
};

class Service {
private:
	int serviceID;
	string serviceName;
	double serviceFee;
public:
	int getID() { return serviceID; }
	string getName() { return serviceName; }
	double getFee() { return serviceFee; }

	Service() :serviceID(0), serviceName(""), serviceFee(0) {}
};

class CASystem {
private:
	Operator curOperator;
	int curOperatorID;
	Member member;
	MemberStatus MS;

public:
	CASystem() {
		curOperator = Operator::NIL;
		curOperatorID = 0;
		MS = MemberStatus::FAIL;
	}
	Operator getCurOperator() { return curOperator; }
	int getOperatorID() { return curOperatorID; }
	ServiceCenter SC;
	vector<Member> members;
	vector<Provider> providers;
	void showInstructions() {
		int i=0;
		printf("You are Logged in as:%s, your ID is:%d\n", operatorToString(curOperator).c_str(),curOperatorID);
		//TODO:Member Info
		printf("You are dealing with the member:\n");
		printf("\n\n");
		// 0
		printf("[%d]:Login & Relogin\n", i); i++;
		printf("[%d]:Login Member\n", i); i++;
		printf("[%d]:Generate Bills\n", i); i++;
		printf("[%d]:Save & Exit\n", i); i++;
		printf("[%d]:Save & Exit\n", i); i++;
		printf("\n\n\n\n");
		// 5
	}
	
public:
	void Run() {
		try {
			MyConnector MC;
			int cmd = 0;
			int manCmd = 1;
			while (1) {
				if (!manCmd) {
					showInstructions();
					scanf("%d", &cmd);
				} else	manCmd = 0;
				switch (cmd) {
				case 0:
					/* Login and get the operator info */
					Login L;
					while ((curOperatorID = L.login()) == NULL) {
						cout << "wrong login information, Try again" << endl;
					}
					/* Login successful, get curOperator */
					MC.res = MC.stmt->executeQuery("select Type from provider where id = " + to_string(curOperatorID));
					if (MC.res->next()) {
						curOperator = stringToOperator(MC.res->getString("type"));
					}
					printf("Login successful! Your ID is :%d %s\n", curOperatorID, operatorToString(curOperator).c_str());
					break;
				case 1://TODO
				{
					/* Read Membercard and verify */
					// Input the member# instead of read it by reading member card 
					int memberNumber;
					string strMemberNumber;
					Verify verify;
					cout << "Input Member Number:";
					cin >> strMemberNumber;
					//TODO:verify....
					//Verify Success
					//print Member information
					if (verify.verify(strMemberNumber) == MemberStatus::FAIL) {
						// Say the reason
					}
				}break;
				case 2:break;
				case 3:break;
				default:break;
				}
			}
		}CATCHSQL;

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
	CC.Run();

	return EXIT_SUCCESS;
}


