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

	double profit = 0.0;
	int number = 0;
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

			m_currentProfit += profit;

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
	transactionData << "��ǰӯ��:��" << m_currentProfit << std::endl;
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
	
	transactionData << "====================================" << std::endl;

	transactionData.close();
}



