#include <iostream>
class Money
{
private:
	int m_dollars;
	int m_cents;
public:
	Money();
	Money(int dollars, int cents);
	Money(std::string money);
	Money Add(int dollars, int cents);
	int GetLeva();
	int GetCoins();
	static std::string GetMoneyAsString(Money money);
	friend std::ostream& operator<<(std::ostream& os, Money money);
};
