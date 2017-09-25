#pragma once

#include "DataType.h"

#include <vector>

class TransactionManager
{
public:
	TransactionManager();
	~TransactionManager();

	void OpenPosition(const std::string& instrumentId, const std::string& date, Position::Type type, double price, int number);
	void ClosePosition(const std::string& instrumentId, const std::string& date, Position::Type closeType, double price);
	void DumpCurrentStatus();

private :
	std::vector<Position>   m_Positions;
	std::vector<double>     m_ProfitLoss;
	std::string             m_transactionDataFilePath;

	int                     m_transactionNumbers;
};

