#pragma once

#include "DataType.h"

#include <vector>

namespace Json
{
	class Value;
}

class TransactionManager
{
public:
	TransactionManager(const Json::Value& historyRoot);
	~TransactionManager();

	void OpenPosition(const std::string& instrumentId, const std::string& date, Position::Type type, double price, int number);
	void ClosePosition(const std::string& instrumentId, const std::string& date, Position::Type closeType, double price, int dayIndex);
	void DumpCurrentStatus();

private :
	std::vector<Position>   m_Positions;
	std::vector<double>     m_ProfitLoss;
	std::vector<int>        m_holdPositionDays;
	std::string             m_transactionDataFilePath;

	int                     m_transactionNumbers;

	const Json::Value&      m_rootHistory;
};

