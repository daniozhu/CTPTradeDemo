#include "stdafx.h"
#include "TransactionManager.h"
#include "DataType.h"

#include <fstream>


TransactionManager::TransactionManager()
	:m_transactionDataFilePath("c:\\temp\\transaction.csv"),
	m_transactionNumbers(0),
	m_currentProfit(0.0)
{
	m_Positions.clear();

	std::ofstream transactionData;
	transactionData.open(m_transactionDataFilePath);
	transactionData << "合约编号" << ","
		<< "日期" << ","
		<< "开/平" << ","
		<< "多/空" << ","
		<< "价格" << ","
		<< "手" << ","
		<< "平仓盈亏" << std::endl;
	transactionData.close();
}


TransactionManager::~TransactionManager()
{
}

void TransactionManager::OpenPosition(const std::string& instrumentId, 
	const std::string& date, 
	Position::Type type, 
	double price, 
	int number)
{
	Position pos;
	pos.PosType = type;
	pos.InstrumentId = instrumentId;
	pos.Number = number;
	pos.Price = price;
	pos.OpenDate = date;

	m_Positions.push_back(std::move(pos));

	++m_transactionNumbers;

	// 打印到屏幕
	std::cout << "开仓 : " << instrumentId.c_str() << ", " 
		<< date.c_str() << ", " 
		<< price << ", "
		<< (type == Position::eBuy ? "多" : "空") << std::endl;

	// 保存交易记录到文件
	std::ofstream transactionData;
	transactionData.open(m_transactionDataFilePath, std::ios::app);
	transactionData << instrumentId.c_str() << ","
		<< date.c_str() << ","
		<< "开" << ","
		<< (type == Position::eBuy ? "多" : "空") << ","
		<< price << ","
		<< number << std::endl;
	transactionData.close();
}

void TransactionManager::ClosePosition(const std::string & instrumentId, 
	const std::string & date, 
	Position::Type closeType, 
	double price)
{
	std::ofstream transactionData;
	transactionData.open(m_transactionDataFilePath, std::ios::app);

	double profit = 0.0;
	int number = 0;
	size_t i = 0;
	while (i < m_Positions.size()) {
		if (m_Positions[i].PosType == closeType) 
		{
			++m_transactionNumbers;

			// 计算盈亏
			double profit = 0.0;

			// 平多仓，　盈亏　＝　当前价格减去开仓价格，乘以数量
			if (closeType == Position::eBuy)
			{
				profit = (price - m_Positions[i].Price) * m_Positions[i].Number;
			}
			//　平空仓，盈亏　＝　开仓价格减去当前价格，乘以数量
			else
			{
				profit = (m_Positions[i].Price - price) * m_Positions[i].Number;
			}

			m_currentProfit += profit;

			// 打印到屏幕
			std::cout << "平仓: " << instrumentId.c_str() << ", "
				<< date.c_str() << ", "
				<< price << ", "
				<< (closeType == Position::eBuy ? "多" : "空") << std::endl;

			// 保存到文件
			transactionData << instrumentId.c_str() << ","
				<< date.c_str() << ","
				<< "平" << ","
				<< (closeType == Position::eBuy ? "多" : "空") << ","
				<< price << ","
				<< m_Positions[i].Number << ","
				<< profit << std::endl;

			m_Positions.erase(m_Positions.begin() + i);

		} 
		else 
		{
			++i;
		}
	}

	transactionData.close();
}

void TransactionManager::DumpCurrentStatus()
{
	std::ofstream transactionData;
	transactionData.open(m_transactionDataFilePath, std::ios::app);

	transactionData << "====================================" << std::endl;
	transactionData << "交易次数: " << m_transactionNumbers << std::endl;
	transactionData << "当前盈亏:　" << m_currentProfit << std::endl;
	transactionData << "当前持仓: " << m_Positions.size() << std::endl;
	if (!m_Positions.empty())
	{
		transactionData <<"合约编号, 手, 开仓日期, 多/空, 开仓价格" << std::endl;
		for (const auto& pos : m_Positions)
		{
			transactionData << pos.InstrumentId.c_str() << "," 
				<< pos.Number << "," 
				<< pos.OpenDate.c_str() << "," 
				<< (pos.PosType == Position::eBuy ? "多" : "空") << ","
				<< pos.Price << std::endl;
		}
	}
	
	transactionData << "====================================" << std::endl;

	transactionData.close();
}



