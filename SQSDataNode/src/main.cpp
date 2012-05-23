#include <iostream>
#include "Database.h"

using namespace std;

int main()
{
	Database db;
	db.createQueue("queueA");
	db.createQueue("queueB");
	db.createQueue("queueC");
	vector<string> list = db.listQueue();
	for (int i = 0; i < list.size(); ++i) {
		cout << list[i] << endl;
	}
	db.deleteQueue("queueC");
	cout << "size: " << db.listQueue().size() << endl;

	int num = -1;
	num = db.putMessage("queueA", "hello world");
	cout << "num is: " << num << endl;
	num = db.putMessage("queueA", "hello world 2", 123);
	cout << "num is: " << num << endl;
	num = db.putMessage("queueA", "hello world 3");
	cout << "num is: " << num << endl;

	bool result;
	cout << "message: " << db.getMessage("queueA", 1, result) << " result: " << result << endl
	     << "message: " << db.getMessage("queueA", 3, result) << " result: " << result << endl;

	cout << "result is: " << db.deleteMessage("queueb", 3) << endl
	     << "result is: " << db.deleteMessage("queueA", 124) << endl;

	return 0;
}