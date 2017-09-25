#include "stdafx.h"
#include "TransactionManager.h"
#include "DataType.h"

#include "../json_cpp/include/json.h"

#include <fstream>
#include <numeric>
#include <algorithm>


TransactionManager::TransactionManager()
	:m_transactionDataFilePath("c:\\temp\\transaction.csv"),
	m_transactionNumbers(0),
	m_closedPositions(0),
	m_profitTimes(0),
	m_coutinousProfitTimes(0),
	m_coutinousLossTimes(0),
	m_totalHoldingDays(0),
	m_currentProfitLoss(0.0),
	m_totalProfit(0.0),
	m_totalLoss(0.0),
	m_maxProfit(0.0),
	m_maxLoss(0.0),
	m_bLastTimeProfit(true),
	m_bIsFirstTimeClose(true)
{
	m_Positions.clear();
	m_continousLossTimesVec.clear();
	m_continousProfitTimesVec.clear();

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
	pos.OpenPrice = price;
	pos.OpenDate = date;
	pos.IsClosed = false;
	pos.CloseDate = "";
	pos.ClosePrice = 0.0;
	pos.HoldingDays = 0;
	pos.ProfitLoss = 0.0;

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

	for (auto& pos : m_Positions)
	{
		if (!pos.IsClosed && pos.PosType == closeType)
		{
			// 计算此合约盈亏
			double profit = 0.0;

			// 平多仓，　盈亏　＝　当前价格减去开仓价格，乘以数量
			if (closeType == Position::eBuy)
			{
				profit = (price - pos.OpenPrice) * pos.Number;
			}
			//　平空仓，盈亏　＝　开仓价格减去当前价格，乘以数量
			else
			{
				profit = (pos.OpenPrice - price) * pos.Number;
			}

			pos.ProfitLoss = profit;
			pos.IsClosed = true;
			pos.CloseDate = date;
			pos.ClosePrice = price;

			m_currentProfitLoss += profit;
			m_totalHoldingDays += pos.HoldingDays;
			++m_transactionNumbers;
			++m_closedPositions;

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
				<< pos.Number << ","
				<< pos.ProfitLoss << std::endl;

			if (profit > 0.0) //此次平仓盈利
			{
				++m_profitTimes;          //增加总盈利次数
				m_totalProfit += profit;  //增加总盈利额

				if (m_bIsFirstTimeClose) //特殊处理第一次平仓盈利情况
				{
					m_maxProfit = profit;
					m_bLastTimeProfit = true;
					++m_coutinousProfitTimes;

					m_bIsFirstTimeClose = false;
					continue;
				}

				if (profit > m_maxProfit) //目前单次平仓最大盈利额
				{
					m_maxProfit = profit;
				}

				if (!m_bLastTimeProfit) // 上次平仓亏损，保存连续亏损次数并重置（因为此次平仓盈利）
				{
					m_continousLossTimesVec.push_back(m_coutinousLossTimes);
					m_coutinousLossTimes = 0;
				}

				++m_coutinousProfitTimes; //增加连续盈利次数
				m_bLastTimeProfit = true; //上次盈利设为true
			}
			else //此次平仓亏损
			{
				m_totalLoss += profit;    //增加总亏损额

				if (m_bIsFirstTimeClose)  //特殊处理第一平仓亏损情况
				{
					m_maxLoss = profit;
					m_bLastTimeProfit = false;
					++m_coutinousLossTimes;

					m_bIsFirstTimeClose = false;
					continue;
				}

				if (profit < m_maxLoss)           //目前单次平仓最大亏损额
				{
					m_maxLoss = profit;
				}

				if (m_bLastTimeProfit)           //上次平仓为盈利，保存连续盈利次数，并重置（因为此次平仓亏损）
				{
					m_continousProfitTimesVec.push_back(m_coutinousProfitTimes);
					m_coutinousProfitTimes = 0;
				}

				++m_coutinousLossTimes;            //增加连续亏损次数
				m_bLastTimeProfit = false;         //上次盈利设为false
			}
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
	transactionData << "当前盈亏:　" << m_currentProfitLoss << std::endl;
	transactionData << "当前持仓: " << m_Positions.size() - m_closedPositions << std::endl;

	transactionData << "胜率： " << ((double)m_profitTimes / m_closedPositions) * 100 << "%" << std::endl;

	transactionData << "盈亏比： " << (m_totalProfit / abs(m_totalLoss))<< std::endl;
	transactionData << "平均盈利： " << (m_totalProfit / m_profitTimes)<< std::endl;
	transactionData << "平均亏损： " << (abs(m_totalLoss) / (m_closedPositions - m_profitTimes)) << std::endl;
	transactionData << "平均持仓天数： " << (double)m_totalHoldingDays / m_closedPositions << std::endl;

	auto iter_max_profit = std::max_element(m_continousProfitTimesVec.begin(), m_continousProfitTimesVec.end());
	auto iter_max_loss = std::max_element(m_continousLossTimesVec.begin(), m_continousLossTimesVec.end());
	transactionData << "连续盈利最大次数： " << *iter_max_profit << std::endl;
	transactionData << "连续盈利平均次数： " << std::accumulate(m_continousProfitTimesVec.begin(), m_continousProfitTimesVec.end(), 0.0) / m_continousProfitTimesVec.size() << std::endl;
	transactionData << "连续亏损最大次数： " << *iter_max_loss << std::endl;
	transactionData << "连续亏损平均次数： " << std::accumulate(m_continousLossTimesVec.begin(), m_continousLossTimesVec.end(), 0.0) / m_continousLossTimesVec.size() << std::endl;
	transactionData << "最大盈利： " << m_maxProfit << std::endl;
	transactionData << "最大亏损: " << m_maxLoss << std::endl;
	transactionData << "====================================" << std::endl;

	transactionData.close();
}

void TransactionManager::IncreaseHoldingDay(int nDay /*=1*/)
{
	for (auto& pos : m_Positions)
	{
		if (!pos.IsClosed)
			pos.HoldingDays += nDay;
	}
}


