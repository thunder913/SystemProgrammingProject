#include "Money.h"
#include <iostream>
#include <string>
#include <format>

using namespace std;

Money::Money(string money) {
	int index = money.find(".");

	string dollar = money.substr(0, index);
	string cent = money.substr(index + 1);
	m_dollars = stoi(dollar);
	m_cents = stoi(cent);
}

Money::Money() {
	m_dollars = 0;
	m_cents = 0;
}

Money::Money(int dollars, int cents)
{
	m_dollars = dollars;
	m_cents = cents;
}

Money Money::Add(int dollars, int cents)
{
	if (m_cents + cents >= 100) {
		int dollarsToAdd = 0;
		while (m_cents + cents > 100) {
			dollarsToAdd++;
			m_cents -= 100;
		}
		return Money(m_dollars + dollars + dollarsToAdd, m_cents + cents);
	}

	return Money(m_dollars + dollars, m_cents + cents);
}

int Money::GetLeva() { return m_dollars; }
int Money::GetCoins() { return m_cents; }

std::ostream& operator<<(ostream& os, Money money)
{
	string moneyString = Money::GetMoneyAsString(money);
	os << moneyString;
	return os;
}

string Money::GetMoneyAsString(Money money) {
	string cents = std::to_string(money.GetCoins());
	if (money.GetCoins() < 10) {
		cents.insert(0, "0");
	}
	string converted = to_string(money.GetLeva()) + "." + cents + "BGN";
	return converted;
}
