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
	pos.Price = price;
	pos.OpenDate = date;

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

	size_t i = 0;
	while (i < m_Positions.size()) {
		if (m_Positions[i].PosType == closeType) 
		{
			++m_transactionNumbers;

			// ����ӯ��
			double profit = 0.0;

			// ƽ��֣���ӯ����������ǰ�۸��ȥ���ּ۸񣬳�������
			if (closeType == Position::eBuy)
			{
				profit = (price - m_Positions[i].Price) * m_Positions[i].Number;
			}
			//��ƽ�ղ֣�ӯ�����������ּ۸��ȥ��ǰ�۸񣬳�������
			else
			{
				profit = (m_Positions[i].Price - price) * m_Positions[i].Number;
			}

			m_ProfitLoss.push_back(profit);

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
	transactionData << "���״���: " << m_transactionNumbers << std::endl;
	transactionData << "��ǰӯ��:��" << std::accumulate(m_ProfitLoss.begin(), m_ProfitLoss.end(), 0.0) << std::endl;
	transactionData << "��ǰ�ֲ�: " << m_Positions.size() << std::endl;
	if (!m_Positions.empty())
	{
		transactionData <<"��Լ���, ��, ��������, ��/��, ���ּ۸�" << std::endl;
		for (const auto& pos : m_Positions)
		{
			transactionData << pos.InstrumentId.c_str() << "," 
				<< pos.Number << "," 
				<< pos.OpenDate.c_str() << "," 
				<< (pos.PosType == Position::eBuy ? "��" : "��") << ","
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

	transactionData << "ʤ�ʣ� " << ((double)profitTimes / m_ProfitLoss.size()) * 100 << "%" << std::endl;
	transactionData << "ӯ���ȣ� " << (profit / abs(loss))<< std::endl;
	transactionData << "ƽ��ӯ���� " << (profit / profitTimes)<< std::endl;
	transactionData << "ƽ������ " << (abs(loss) / (m_ProfitLoss.size() - profitTimes)) << std::endl;

	auto iter_max_profit = std::max_element(continousProfitDaysVec.begin(), continousProfitDaysVec.end());
	auto iter_max_loss = std::max_element(continousLossDaysVec.begin(), continousLossDaysVec.end());
	transactionData << "����ӯ����������� " << *iter_max_profit << std::endl;
	transactionData << "����ӯ��ƽ�������� " << std::accumulate(continousProfitDaysVec.begin(), continousProfitDaysVec.end(), 0) / continousProfitDaysVec.size() << std::endl;
	transactionData << "����������������� " << *iter_max_loss << std::endl;
	transactionData << "��������ƽ�������� " << std::accumulate(continousLossDaysVec.begin(), continousLossDaysVec.end(), 0) / continousLossDaysVec.size() << std::endl;
	
	transactionData << "====================================" << std::endl;

	transactionData.close();
}


