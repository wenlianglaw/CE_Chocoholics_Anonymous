#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <string.h>
#include <conio.h>
#include <string>
#include <time.h>
#include <regex>
#include <sstream>

#include "mysql++.h"

using namespace mysqlpp;
using namespace std;

#define CATCHSQL catch (exception &e) {\
				cout << "# ERR: SQLException in " << __FILE__;\
				cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;\
				cout << "# ERR: " << e.what();\
				system("pause");\
				}

enum Operator { NIL = 0x00,PROVIDER = 0x100, MEMBER=0x01,MANAGER=0x10,ALL = 0x11111111 };
enum MemberStatus { FAIL = 0x00 ,VALIDATED, MEMBER_SUSPENDED, SUSPENDED };
enum BillStatus {NOT_PAID = 0x00, PAID};



/* Auxiliary Functions */
Operator stringToOperator(const mysqlpp::String &str) {
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
	Connection conn;
	StoreQueryResult res;
	MyConnector() {
		try {
			// Connect to the sample database.
			if (!conn.connect("chocoholics", "127.0.0.1:3306",
				"root", "123456"))
				cerr << "DB connection failed: " << conn.error() << endl;				
		}CATCHSQL;
	}
	~MyConnector() {
		
	}

	void example() {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it.
		Query query = conn.query("select * from service_directory");
		if (StoreQueryResult res = query.store()) {
			cout << "We have:" << endl;
			StoreQueryResult::const_iterator it;
			for (it = res.begin(); it != res.end(); ++it) {
				Row row = *it;
				cout << '\t' << row[0] << row[1] << row[2] << endl;
			}
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
		}
	}
	
};


class Member {
private:
	string name;
	unsigned int memberID;
public:
	string getName() { return name; }
	unsigned int getMemberID() { return memberID; }

	Member(string name, int number) {
		this->name = name;
		memberID = number;
	}
	Member() :name(string()), memberID(0) {}
};

class Provider {
private:
	string name;
	unsigned int providerId;
public:
	string getName() { return name; }
	unsigned int getProviderID() { return providerId; }
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

	
	string encryptPWD(const string &pwd)const {
		// TODO:: MD5 or other algorithm
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
		// ch != Enter
		while (ch != 13) {
			//Backward
			if (ch == 8) {
				if (!password.empty()) {
					password.pop_back();
					printf("%c %c", 8, 8);
				}
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
			Query query = MC.conn.query("select * from provider where Name = '" + username + "'");
			StoreQueryResult res = query.store();
			if (res.size()) {
				String dbPWD = res[0]["pwd"];
				if (dbPWD == password) {
					int id = atoi(res[0]["id"].c_str());
					return id;
				}
			}
		}CATCHSQL;
		return 0;
	}
};

/**
	Verify a member's status
	Input memberNumber and get this member's info
*/
class Verify{
public:
	MemberStatus stringToMS(const mysqlpp::String &str)const {
		if (str == "VALIDATED")
			return VALIDATED;
		if (str == "MEMBER_SUSPENDED")
			return MEMBER_SUSPENDED;
		if (str == "SUSPENDED")
			return SUSPENDED;
		return MemberStatus::FAIL;
	}
	int checkFormat(const string &strNumber)const {
		for (auto ch : strNumber) {
			if (ch < '0' || ch > '9') return 0;
		}
		return 1;
	}
	MemberStatus verify(const string& strNumber)const {
		if (!checkFormat(strNumber)) {
			return FAIL;
		}
		unsigned int memberNumber = stoi(strNumber.substr(0,9));
		try {
			MyConnector MC;
			// Look up in the DB, get the status
			Query query = MC.conn.query("select status from member where id = " + to_string(memberNumber));
			StoreQueryResult res = query.store();
			if (res.size()) {
				MemberStatus MS = stringToMS(res[0][0]);
				return MS;
			}else {
				cout << "No matched Data" << endl;
			}
		}CATCHSQL;
		return MemberStatus::FAIL;
	}
	
};

class Service {
private:
	unsigned int serviceID;
	string serviceName;
	double serviceFee;
public:
	unsigned int getID()const { return serviceID; }
	string getName()const { return serviceName; }
	double getFee()const { return serviceFee; }
	static Service qureyService(unsigned int serviceNumber) {
		try {
			MyConnector MC;
			Service service;
			Query query = MC.conn.query("select * from service_directory where id = " + to_string(serviceNumber));
			StoreQueryResult res = query.store();
			if (res.size()) {
				service.serviceID = stoi(string(res[0][0]).substr(0,9));
				service.serviceName=string(res[0][1]).substr(0, 9);
				service.serviceFee = stod(string(res[0][2]).substr(0, 9));
			}else {
				cout << "No matched Data" << endl;
			}
			return service;
		}CATCHSQL;
	}
	Service() :serviceID(0), serviceName(""), serviceFee(0) {}
};

class Bill :public Service, public Provider, public Member {
private:
	string strComments;
	string curTime;
	string dateSProvided;
	BillStatus billStatus;

public:
	void writeComments() {}
	void setCurTime(const string &ct) { curTime = ct; }
	void setDateSProvided(const string &dt) { dateSProvided = dt; }
	void setBillStatus() {}

	string getComments()const { return strComments; }
	string getCurTime()const { return curTime; }
	string getDateProvided()const { return dateSProvided; }
	BillStatus getBillStatus()const { return billStatus; }

	static int addBillToDB(const Bill &bill) {}


};

class CASystem {
private:
	Operator curOperator;
	unsigned int curOperatorID;
	Member member;
	MemberStatus MS;
	string strFront;
	string strEnd;
public:
	CASystem() {
		curOperator = Operator::NIL;
		curOperatorID = 0;
		MS = MemberStatus::FAIL;
		strFront = string();
		strEnd = string();
	}
	Operator getCurOperator() { return curOperator; }
	unsigned int getOperatorID() { return curOperatorID; }
	vector<Member> members;
	vector<Provider> providers;
	void showInstructions() {
		unsigned int i=0;
		printf("%s,ID:%d\n", operatorToString(curOperator).c_str(),curOperatorID);
		if (!strFront.empty()) {  cout << strFront << endl; strFront = string();}
		//TODO:Member Info
		printf("Member:\n\n");
		printf("\n\n");
		// 0
		printf("[%d]:Login & Relogin\n", i); i++;
		printf("[%d]:Login Member\n", i); i++;
		printf("[%d]:Generate Bills\n", i); i++;
		printf("[%d]:Query Service\n", i); i++;
		printf("[%d]:Save & Exit\n", i); i++;
		printf("\n\n\n\n");
		if (!strEnd.empty()) {  cout << strEnd << endl; strEnd = string(); }
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
					string strCmd;
					bool bGetinput = true;
					while (bGetinput) {
						getline(cin, strCmd);
						strCmd = strCmd.substr(0, 3);
						regex regNumbers("[[:d:]]+");
						if (regex_match(strCmd, regNumbers)) {
							cmd = stoi(strCmd);
							bGetinput = false;
						}
					}
				} else	manCmd = 0;
				switch (cmd) {
				case 0: {
					/* Login and get the operator info */
					Login L;
					while ((curOperatorID = L.login()) == NULL) {
						cout << "wrong login information, Try again" << endl;
					}
					/* Login successful, get curOperator */
					Query query = MC.conn.query("select Type from provider where id = " + to_string(curOperatorID));
					StoreQueryResult res = query.store();
					if (res.size()) {
						curOperator = stringToOperator(res[0]["type"]);
					}else {

					}
					strFront.assign("Login successful!\n");
				}
					break;
				case 1:{
					//TODO
					// Input member info
					if (!grant(Operator::MANAGER | Operator::PROVIDER))  break;
					/* Read Membercard and verify */
					// Input the member# instead of read it by reading member card 
					unsigned int memberNumber;
					string strMemberNumber;
					Verify verify;
					cout << "Input Member Number:";
					cin >> strMemberNumber;
					//TODO:verify....
					//Verify Success
					//print Member information
					if (verify.verify(strMemberNumber) == MemberStatus::FAIL) {
						strEnd.assign("Due to some reason, this member is not logged in\n");
						// say the reason and put up some solutions.
					}
				}break;
				case 2:
				{
					// Create Bill
					Bill bill;
					// MM-DD-YY			%m-%d-%Y
					// HH:MM:SS			%H:%M:%S
					// Get Current Time
					time_t rawtime;
					struct tm *timeinfo;
					time(&rawtime);
					timeinfo = localtime(&rawtime);
					char curTime[80];
					strftime(curTime,80, "%m-%d-%Y %H:%M:%S", timeinfo);
					bill.setCurTime(string(curTime));
					cout << "Current time is: " << curTime << endl;
					// Read date was provided
					cout << "Input the date service was provided in the format MM-DD-YYYY:" << endl;
					string dateProvided = "";
					getline(cin, dateProvided);
					regex legalDate("[[:d:]]{2}-[[:d:]]{2}-[[:d:]]{4}");
					while(dateProvided != "-1") {
						if (regex_match(dateProvided, legalDate)) {
							// TODO: should verify the date, for example, Feburary doesn't have 30 days. The date cannot be someday in the furture ect.
							bill.setDateSProvided(dateProvided);
							break;
						} else {
							cout << "wrong format, please input the date in format MM-DD-YYYY, if you want to cancel, input -1." << endl;
							getline(cin, dateProvided);
						}
					}
					if(bill.get)
					
				}break;
				case 3: {
					// Query Services
					StoreQueryResult res;
					//grant(Operator::MANAGER | PROVIDER);
					string pattern;
					cin >> pattern;
					regex digits("[[:digit:]]+");
					regex name("([[:w:]]|[[:s:]]|[*.%])+");
					if (regex_match(pattern, digits)) {
						// Search by Service ID
						Query query = MC.conn.query("select * from service_directory where id = " + pattern);
						MC.res = query.store();
					}else if (regex_match(pattern, name)) {
						// Search by Service Name
						Query query = MC.conn.query("select * from service_directory where name like '%" + pattern + "%'");
						MC.res = query.store();
					} else {
						strEnd.assign("wrong input.\n");
					}
					if (MC.res.size()) {
						// Print all
						//id
						cout << std::setw(12)<<std::left<< MC.res.field_name(0);
						//Name
						cout << std::setw(30) << std::left << MC.res.field_name(1);
						//Price
						cout << std::setw(12) << std::left << MC.res.field_name(2);
						for (unsigned int i = 3; i < MC.res.num_fields(); i++) {
							cout << std::setw(18) << std::left << MC.res.field_name(i);
						}
						cout << endl;
						for (auto it : MC.res) {
							cout << std::setw(12) << std::left << it[0];
							cout << std::setw(30) << std::left << it[1];
							cout << std::setw(12) << std::left << it[2];
							for (unsigned int i = 3; i < MC.res.num_fields(); i++) {
								cout << std::setw(18)<<std::left
									<<it[i];
							}
							cout << endl;
						}
					} else {
						strEnd.assign("Couldn't find the matched service\n");
					}
				}
					break;						
				default:
					manCmd = 0;
					break;
				}
				//system("cls");
			}//while(1)
		}CATCHSQL;

	}

	// Grant this function with compLvl and compare with curOperator
	int grant(int compLvl) {
		if (compLvl & curOperator) return 1;
		printf("You are not allowed to opearte this function\n");
		return 0;
	}
protected:
};




int main() {
	CASystem CC;	
	CC.Run();


	return EXIT_SUCCESS;
}


