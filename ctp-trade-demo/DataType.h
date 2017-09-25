#pragma once

struct MAData
{
	std::string   Date;
	double        MA5;
	double        MA10;
};

struct Position
{
	enum Type { eBuy, eSell };

	std::string    InstrumentId;
	std::string    OpenDate;
	std::string    CloseDate;
	Type           PosType;
	int            Number;
	double         OpenPrice;
	double         ClosePrice;
	double         ProfitLoss;
	int            HoldingDays;
	bool           IsClosed;
};