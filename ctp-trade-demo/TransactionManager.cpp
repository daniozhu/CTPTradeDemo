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
	transactionData << "��Լ���" << ","
		<< "����" << ","
		<< "��/ƽ" << ","
		<< "��/��" << ","
		<< "�۸�" << ","
		<< "��" << ","
		<< "ƽ��ӯ��" << std::endl;
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

	// ��ӡ����Ļ
	std::cout << "���� : " << instrumentId.c_str() << ", " 
		<< date.c_str() << ", " 
		<< price << ", "
		<< (type == Position::eBuy ? "��" : "��") << std::endl;

	// ���潻�׼�¼���ļ�
	std::ofstream transactionData;
	transactionData.open(m_transactionDataFilePath, std::ios::app);
	transactionData << instrumentId.c_str() << ","
		<< date.c_str() << ","
		<< "��" << ","
		<< (type == Position::eBuy ? "��" : "��") << ","
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
			// ����˺�Լӯ��
			double profit = 0.0;

			// ƽ��֣���ӯ����������ǰ�۸��ȥ���ּ۸񣬳�������
			if (closeType == Position::eBuy)
			{
				profit = (price - pos.OpenPrice) * pos.Number;
			}
			//��ƽ�ղ֣�ӯ�����������ּ۸��ȥ��ǰ�۸񣬳�������
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

			// ��ӡ����Ļ
			std::cout << "ƽ��: " << instrumentId.c_str() << ", "
				<< date.c_str() << ", "
				<< price << ", "
				<< (closeType == Position::eBuy ? "��" : "��") << std::endl;

			// ���浽�ļ�
			transactionData << instrumentId.c_str() << ","
				<< date.c_str() << ","
				<< "ƽ" << ","
				<< (closeType == Position::eBuy ? "��" : "��") << ","
				<< price << ","
				<< pos.Number << ","
				<< pos.ProfitLoss << std::endl;

			if (profit > 0.0) //�˴�ƽ��ӯ��
			{
				++m_profitTimes;          //������ӯ������
				m_totalProfit += profit;  //������ӯ����

				if (m_bIsFirstTimeClose) //���⴦���һ��ƽ��ӯ�����
				{
					m_maxProfit = profit;
					m_bLastTimeProfit = true;
					++m_coutinousProfitTimes;

					m_bIsFirstTimeClose = false;
					continue;
				}

				if (profit > m_maxProfit) //Ŀǰ����ƽ�����ӯ����
				{
					m_maxProfit = profit;
				}

				if (!m_bLastTimeProfit) // �ϴ�ƽ�ֿ��𣬱�������������������ã���Ϊ�˴�ƽ��ӯ����
				{
					m_continousLossTimesVec.push_back(m_coutinousLossTimes);
					m_coutinousLossTimes = 0;
				}

				++m_coutinousProfitTimes; //��������ӯ������
				m_bLastTimeProfit = true; //�ϴ�ӯ����Ϊtrue
			}
			else //�˴�ƽ�ֿ���
			{
				m_totalLoss += profit;    //�����ܿ����

				if (m_bIsFirstTimeClose)  //���⴦���һƽ�ֿ������
				{
					m_maxLoss = profit;
					m_bLastTimeProfit = false;
					++m_coutinousLossTimes;

					m_bIsFirstTimeClose = false;
					continue;
				}

				if (profit < m_maxLoss)           //Ŀǰ����ƽ���������
				{
					m_maxLoss = profit;
				}

				if (m_bLastTimeProfit)           //�ϴ�ƽ��Ϊӯ������������ӯ�������������ã���Ϊ�˴�ƽ�ֿ���
				{
					m_continousProfitTimesVec.push_back(m_coutinousProfitTimes);
					m_coutinousProfitTimes = 0;
				}

				++m_coutinousLossTimes;            //���������������
				m_bLastTimeProfit = false;         //�ϴ�ӯ����Ϊfalse
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
	transactionData << "���״���: " << m_transactionNumbers << std::endl;
	transactionData << "��ǰӯ��:��" << m_currentProfitLoss << std::endl;
	transactionData << "��ǰ�ֲ�: " << m_Positions.size() - m_closedPositions << std::endl;

	transactionData << "ʤ�ʣ� " << ((double)m_profitTimes / m_closedPositions) * 100 << "%" << std::endl;

	transactionData << "ӯ���ȣ� " << (m_totalProfit / abs(m_totalLoss))<< std::endl;
	transactionData << "ƽ��ӯ���� " << (m_totalProfit / m_profitTimes)<< std::endl;
	transactionData << "ƽ������ " << (abs(m_totalLoss) / (m_closedPositions - m_profitTimes)) << std::endl;
	transactionData << "ƽ���ֲ������� " << (double)m_totalHoldingDays / m_closedPositions << std::endl;

	auto iter_max_profit = std::max_element(m_continousProfitTimesVec.begin(), m_continousProfitTimesVec.end());
	auto iter_max_loss = std::max_element(m_continousLossTimesVec.begin(), m_continousLossTimesVec.end());
	transactionData << "����ӯ���������� " << *iter_max_profit << std::endl;
	transactionData << "����ӯ��ƽ�������� " << std::accumulate(m_continousProfitTimesVec.begin(), m_continousProfitTimesVec.end(), 0.0) / m_continousProfitTimesVec.size() << std::endl;
	transactionData << "���������������� " << *iter_max_loss << std::endl;
	transactionData << "��������ƽ�������� " << std::accumulate(m_continousLossTimesVec.begin(), m_continousLossTimesVec.end(), 0.0) / m_continousLossTimesVec.size() << std::endl;
	transactionData << "���ӯ���� " << m_maxProfit << std::endl;
	transactionData << "������: " << m_maxLoss << std::endl;
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


