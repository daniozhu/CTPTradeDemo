#include "stdafx.h"
#include "TransactionManager.h"
#include "DataType.h"

#include <fstream>
#include <numeric>
#include <algorithm>


TransactionManager::TransactionManager()
	:m_transactionDataFilePath("c:\\temp\\transaction.csv"),
	m_transactionNumbers(0)
{
	m_Positions.clear();
	m_ProfitLoss.clear();

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

			m_ProfitLoss.push_back(profit);

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
	transactionData << "当前盈亏:　" << std::accumulate(m_ProfitLoss.begin(), m_ProfitLoss.end(), 0.0) << std::endl;
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

	int profitTimes = 0;
	double profit = 0.0;
	double loss = 0.0;
	std::vector<int> continousProfitDaysVec;
	std::vector<int> continousLossDaysVec;
	int coutinousProfitDays = 0;
	int coutinousLossDays = 0;
	bool bLastDayProfit = false;
	for (size_t i= 0; i < m_ProfitLoss.size(); ++i)
	{
		const double value = m_ProfitLoss[i];

		if (value > 0.0)
		{
			++profitTimes;
			profit += value;

			if (0 == i)
			{
				bLastDayProfit = true;
				++coutinousProfitDays;
				continue;
			}

			if (!bLastDayProfit)
			{
				continousLossDaysVec.push_back(coutinousLossDays);
				coutinousLossDays = 0;
			}

			++coutinousProfitDays;
			bLastDayProfit = true;
		}
		else
		{
			loss += value;

			if (0 == i)
			{
				bLastDayProfit = false;
				++coutinousLossDays;
				continue;
			}

			if (bLastDayProfit)
			{
				continousProfitDaysVec.push_back(coutinousProfitDays);
				coutinousProfitDays = 0;
			}

			++coutinousLossDays;
			bLastDayProfit = false;
		}
	}

	transactionData << "胜率： " << ((double)profitTimes / m_ProfitLoss.size()) * 100 << "%" << std::endl;
	transactionData << "盈亏比： " << (profit / abs(loss))<< std::endl;
	transactionData << "平均盈利： " << (profit / profitTimes)<< std::endl;
	transactionData << "平均亏损： " << (abs(loss) / (m_ProfitLoss.size() - profitTimes)) << std::endl;

	auto iter_max_profit = std::max_element(continousProfitDaysVec.begin(), continousProfitDaysVec.end());
	auto iter_max_loss = std::max_element(continousLossDaysVec.begin(), continousLossDaysVec.end());
	transactionData << "连续盈利最大天数： " << *iter_max_profit << std::endl;
	transactionData << "连续盈利平均天数： " << std::accumulate(continousProfitDaysVec.begin(), continousProfitDaysVec.end(), 0) / continousProfitDaysVec.size() << std::endl;
	transactionData << "连续亏损最大天数： " << *iter_max_loss << std::endl;
	transactionData << "连续亏损平均天数： " << std::accumulate(continousLossDaysVec.begin(), continousLossDaysVec.end(), 0) / continousLossDaysVec.size() << std::endl;
	
	transactionData << "====================================" << std::endl;

	transactionData.close();
}


