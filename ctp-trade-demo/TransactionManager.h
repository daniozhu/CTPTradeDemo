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
	std::vector<int>        m_continousProfitTimesVec; //����ӯ����������
	std::vector<int>        m_continousLossTimesVec;   //���������������
	std::string             m_transactionDataFilePath;
	int                     m_transactionNumbers;      //���״��� ������ + ƽ�֣�
	int                     m_closedPositions;         //ƽ�ִ���
	int                     m_profitTimes;             //ӯ������
	int                     m_coutinousProfitTimes;    //��ǰ����ӯ������
	int                     m_coutinousLossTimes;      //��ǰ�����������
	int                     m_totalHoldingDays;        //���к�Լ�ֲܳ�����
	double                  m_currentProfitLoss;       //��ǰӯ��
	double                  m_totalProfit;             //��ӯ��
	double                  m_totalLoss;               //�ܿ���
	double                  m_maxProfit;               //���ӯ��
	double                  m_maxLoss;                 //������
	bool                    m_bLastTimeProfit;         //�ϴ�ƽ���Ƿ�ӯ��
	bool                    m_bIsFirstTimeClose;       //�Ƿ��һ��ƽ��
};

