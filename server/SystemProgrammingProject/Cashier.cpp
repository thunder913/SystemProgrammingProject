#include "Cashier.h"
#include <map>
#include <mutex>
using namespace std;
static mutex mtx;
Cashier::Cashier() {
	this->billsAvailable = {
	{ "0.01", 0 },
	{ "0.02", 0 },
	{ "0.05", 0 },
	{ "0.10", 0 },
	{ "0.20", 0 },
	{ "0.50", 0 },
	{ "1.00", 0 },
	{ "2.00", 0 },
	{ "5.00", 0 },
	{ "10.00", 0 },
	{ "20.00", 0 },
	{ "50.00", 0 },
	{ "100.00", 0 },
	};

	SetDefaultBillsToReturn();
}

static map<string, int, cmpByBiggestValue> billsToReturn;
static Money totalChange = Money();
static Money paid = Money();
string Cashier::Pay(Money bill, string paidBills)
{
	std::lock_guard<std::mutex> lck{ mtx };
	map<string, int, cmpByBiggestValue> startingBills(billsAvailable);
	map<string, int> getParsedBill = GetParsedBills(paidBills);
	// Calculating the change needed
	int leva = paid.GetLeva() - bill.GetLeva();
	int coins = paid.GetCoins() - bill.GetCoins();


	if (coins < 0) {
		leva--;
		coins += 100;
	}
	Money toReturn = Money(leva, coins);

	// Checking if the cashier has been given less
	if (toReturn.GetLeva() < 0 || toReturn.GetCoins() < 0) {
		//cout << "BILL -- " << bill << " PAID -- " << paid << "--" << paidBills << " - " << bill << "\n";
		SetDefaultBillsToReturn();
		this->billsAvailable = startingBills;
		cout << " -Cashier: I have been scammed, the client gave me less money, so I don't take it!\n";
		return "You have given me less money, that I wanted!";
	}

	// Starting the process of returning the money
	cout << " -Cashier: Received " << paid << " and the bill is " << bill << ". I gotta return " << toReturn << "\n";
	if (toReturn.GetLeva() == 0 && toReturn.GetCoins() == 0) {
		return "Exact amount, nice!";
	}
	if (GetCorrectChange(toReturn)){
		cout << " -Cashier: Returned " << totalChange << " as follows:\n";
		PrintAndResetChange();
	}
	else {
		this->billsAvailable = startingBills;
		totalChange = Money();
		SetDefaultBillsToReturn();
		return "I did not have enough money to return you, come back at later time!";
	}

	// Reseting the default values
	totalChange = Money();
	SetDefaultBillsToReturn();

	return "Received: " + Money::GetMoneyAsString(Money(toReturn));
}

bool Cashier::GetCorrectChange(Money toReturn) {
	// Calculate the current leva, because this is recursive
	int actualLeva = toReturn.GetLeva() - totalChange.GetLeva();
	int actualCoins = toReturn.GetCoins() - totalChange.GetCoins();
	if (actualCoins < 0) {
		actualLeva--;
		actualCoins += 100;
	}
	// Get leva and Coins change
	GetLevaChange(actualLeva);
	GetCoinsChange(actualCoins);
	// Check if the money is still not enough
	if (totalChange.GetLeva() < toReturn.GetLeva()) {
		int levaToReturn = toReturn.GetLeva() - totalChange.GetLeva();
		GetCoinsChange(levaToReturn * 100);

		if (totalChange.GetLeva() < toReturn.GetLeva()) {
			cout << "   -Cashier: Not enough money in the register!\n";
		}
	}

	// If the money is not enough, then go to the bank
	if (totalChange.GetLeva() < toReturn.GetLeva() || totalChange.GetCoins() < toReturn.GetCoins())
	{
		// Checking if the cashier has 100 or 50 leva bills
		bool changedMoney = false;
		if (this->billsAvailable["100.00"] > 0) {
			changedMoney = true;
			this->billsAvailable["100.00"]--;
			this->billsAvailable["20.00"]+=1;
			this->billsAvailable["10.00"]+=2;
			this->billsAvailable["5.00"]+=4;
		}
		else if (this->billsAvailable["50.00"] > 0) {
			changedMoney = true;
			this->billsAvailable["50.00"]--;
			this->billsAvailable["5.00"] += 2;
		}

		if (changedMoney) {
			cout << "   -Cashier: Going to the bank to exchange money \n";
			this->billsAvailable["2.00"] += 10;
			this->billsAvailable["1.00"] += 10;
			this->billsAvailable["0.50"] += 10;
			this->billsAvailable["0.20"] += 10;
			this->billsAvailable["0.10"] += 15;
			this->billsAvailable["0.05"] += 20;
			this->billsAvailable["0.02"] += 20;
			this->billsAvailable["0.01"] += 10;
			GetCorrectChange(toReturn);
		}
		else {
			cout << "   -Cashier: Not enough money!\n";
			return false;
		}
	}
	return true;
}

void Cashier::GetLevaChange(int dollars) {
	if (dollars == 0) {
		return;
	}
	if (dollars >= 100 && this->billsAvailable["100.00"] > 0) {
		AddMoneyToExchange("100.00", true);
		GetLevaChange(dollars - 100);
	}
	else if (dollars >= 50 && this->billsAvailable["50.00"] > 0) {
		AddMoneyToExchange("50.00", true);
		GetLevaChange(dollars - 50);
	}
	else if (dollars >= 20 && this->billsAvailable["20.00"] > 0) {
		AddMoneyToExchange("20.00", true);
		GetLevaChange(dollars - 20);
	}
	else if (dollars >= 10 && this->billsAvailable["10.00"] > 0) {
		AddMoneyToExchange("10.00", true);
		GetLevaChange(dollars - 10);
	}
	else if (dollars >= 5 && this->billsAvailable["5.00"] > 0) {
		AddMoneyToExchange("5.00", true);
		GetLevaChange(dollars - 5);
	}
	else if (dollars >= 2 && this->billsAvailable["2.00"] > 0) {
		AddMoneyToExchange("2.00", true);
		GetLevaChange(dollars - 2);
	}
	else if (dollars >= 1 && this->billsAvailable["1.00"] > 0) {
		AddMoneyToExchange("1.00", true);
		GetLevaChange(dollars - 1);
	}
}

void Cashier::GetCoinsChange(int cents) {
	if (cents == 0) {
		return;
	}
	else if (cents >= 50 && this->billsAvailable["0.50"] > 0)
	{
		AddMoneyToExchange("0.50", false);
		GetCoinsChange(cents - 50);
	}
	else if (cents >= 20 && this->billsAvailable["0.20"] > 0)
	{
		AddMoneyToExchange("0.20", false);
		GetCoinsChange(cents - 20);
	}
	else if (cents >= 10 && this->billsAvailable["0.10"] > 0)
	{
		AddMoneyToExchange("0.10", false);
		GetCoinsChange(cents - 10);
	}
	else if (cents >= 5 && this->billsAvailable["0.05"] > 0)
	{
		AddMoneyToExchange("0.05", false);
		GetCoinsChange(cents - 5);
	}
	else if (cents >= 2 && this->billsAvailable["0.02"] > 0)
	{
		AddMoneyToExchange("0.02", false);
		GetCoinsChange(cents - 2);
	}
	else if (cents >= 1 && this->billsAvailable["0.01"] > 0)
	{
		AddMoneyToExchange("0.01", false);
		GetCoinsChange(cents - 1);
	}
}

void Cashier::PrintAndResetChange() {

	for (auto& kv : billsToReturn) {
		if (kv.second > 0) {
			cout << kv.first << " - " << kv.second << ", ";
		}
	}
	cout << "\n";
}

void Cashier::AddMoneyToExchange(string moneyString, bool isLeva)
{
	billsToReturn[moneyString]++;
	this->billsAvailable[moneyString]--;
	if (isLeva) {
		int money = stoi(moneyString);
		totalChange = totalChange.Add(money, 0);
	}
	else {
		int money = (int)(stof(moneyString) * 100);
		totalChange = totalChange.Add(0, money);
	}
}

void Cashier::SetDefaultBillsToReturn() {
	billsToReturn = {
	{ "0.01", 0 },
	{ "0.02", 0 },
	{ "0.05", 0 },
	{ "0.10", 0 },
	{ "0.20", 0 },
	{ "0.50", 0 },
	{ "1.00", 0 },
	{ "2.00", 0 },
	{ "5.00", 0 },
	{ "10.00", 0 },
	{ "20.00", 0 },
	{ "50.00", 0 },
	{ "100.00", 0 },
	};
	paid = Money();
}



map<string, int> Cashier::GetParsedBills(string bills) {
	map<string, int> toReturn = {
	{ "0.01", 0 },
	{ "0.02", 0 },
	{ "0.05", 0 },
	{ "0.10", 0 },
	{ "0.20", 0 },
	{ "0.50", 0 },
	{ "1.00", 0 },
	{ "2.00", 0 },
	{ "5.00", 0 },
	{ "10.00", 0 },
	{ "20.00", 0 },
	{ "50.00", 0 },
	{ "100.00", 0 },
	};

	while (bills.length()>0)
	{
		int index = bills.find(",");
		string current = bills.substr(0, index);
		bills = bills.substr(index+1);

		int dashIndex = current.find("-");
		string key = current.substr(0, dashIndex);
		string value = current.substr(dashIndex + 1, current.length() - dashIndex);

		int valueInt = stoi(value);
		toReturn[key] = valueInt;

		float moneyValue = stof(key);
		int moneyInt = 0;
		if (moneyValue >= 1) {
			moneyInt = (int) moneyValue;
			paid = paid.Add(moneyInt* valueInt, 0);
		}
		else {
			moneyInt = (int)(moneyValue * 100);
			paid = paid.Add(0, moneyInt* valueInt);
		}
		this->billsAvailable[key] += valueInt;

	}

	return toReturn;
}

void Cashier::PrintDailyMoney(int day) {
	cout << "\nBills available at the start of day " << to_string(day) << ":\n";
	Money totalMoney = Money();
	for (auto& kv : this->billsAvailable) {
			cout << kv.first << " - " << kv.second << ", ";

			float moneyValue = stof(kv.first);
			int moneyInt = 0;
			if (moneyValue >= 1) {
				moneyInt = (int)moneyValue;
				totalMoney = totalMoney.Add(moneyInt * kv.second, 0);
			}
			else {
				moneyInt = (int)(moneyValue * 100);
				totalMoney = totalMoney.Add(0, moneyInt * kv.second);
			}
	}
	cout << "\nI have a total of " << totalMoney << "\n\n";
}