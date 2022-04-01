#pragma once
#include"Money.h"
#include <string>
#include <map>
struct cmpByBiggestValue {
	bool operator()(const std::string& a, const std::string& b) const {
		return stof(a) > stof(b);
	}
};

class Cashier {
private:
	bool GetCorrectChange(Money toReturn);
	void GetLevaChange(int dollars);
	void GetCoinsChange(int cents);
	void PrintChange();
	void AddMoneyToExchange(std::string moneyString, bool isLeva);
	void SetDefaultBillsToReturn();
	std::map<std::string, int, cmpByBiggestValue> billsAvailable;
	std::map<std::string, int> GetParsedBills(std::string bills);
public:
	std::string Pay(Money bill, std::string paidBills);
	Cashier();
	void PrintDailyMoney(int day);
};