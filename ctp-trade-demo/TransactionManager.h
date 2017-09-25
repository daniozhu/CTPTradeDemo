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
	void IncreaseHoldingDay(int nDay = 1);

private :
	std::vector<Position>   m_Positions;
	std::vector<int>        m_continousProfitTimesVec; //连续盈利此数集合
	std::vector<int>        m_continousLossTimesVec;   //连续亏损次数集合
	std::string             m_transactionDataFilePath;
	int                     m_transactionNumbers;      //交易次数 （开仓 + 平仓）
	int                     m_closedPositions;         //平仓次数
	int                     m_profitTimes;             //盈利次数
	int                     m_coutinousProfitTimes;    //当前连续盈利次数
	int                     m_coutinousLossTimes;      //当前连续亏损次数
	int                     m_totalHoldingDays;        //所有合约总持仓天数
	double                  m_currentProfitLoss;       //当前盈亏
	double                  m_totalProfit;             //总盈利
	double                  m_totalLoss;               //总亏损
	double                  m_maxProfit;               //最大盈利
	double                  m_maxLoss;                 //最大亏损
	bool                    m_bLastTimeProfit;         //上次平仓是否盈利
	bool                    m_bIsFirstTimeClose;       //是否第一次平仓
};

