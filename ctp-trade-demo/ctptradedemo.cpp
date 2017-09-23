// ctptradedemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../tradeapi_x64/ThostFtdcTraderApi.h"
#include "../json_cpp/include/json.h"

#include "QCTPTradingSpi.h"
#include "DataType.h"
#include "TransactionManager.h"
#include "Util.h"

#include <fstream>

// 7x24:       180.168.146.187:10030
// standard:   180.168.146.187:10001
char* Trade_Front = "tcp://180.168.146.187:10001";

const int MAX_PATH = 256;
const double EQUAL_TOLERANCE = 0.0000001;

//ע�⣺�������˲ƾ����ص����ݲ��Ǹ�ʽ����JSON���ݣ� JsonCPP�޷�ʶ����Ҫ�Ƚ����е�Key��������š�
//http://stock.finance.sina.com.cn/futures/api/json.php/InnerFuturesService.getInnerFuturesDailyKLine?symbol=RB1710

using InstrumentIdAndHistoryDataPath = std::pair<std::string, std::string>;

namespace
{
	void InputInstrumentIdPath(InstrumentIdAndHistoryDataPath& instrumentIdData)
	{
		std::cout << "�����Լ��ţ�" << std::endl;
		char instrumentId[MAX_PATH];
		std::cin.getline(instrumentId, MAX_PATH);

		std::cout << "������ʷ����·����" << std::endl;
		char historyDataPath[MAX_PATH];
		std::cin.getline(historyDataPath, MAX_PATH);

		instrumentIdData.first = instrumentId;
		instrumentIdData.second = historyDataPath;
	}
}

int main()
{	
	char ch = '0';
	std::cout << "***************************************************" << std::endl;
	std::cout << "a: ��ʽ����ʷ���ݣ�JSON�� �ļ�" << std::endl;
	std::cout << "b: ����ʷ���ݻز����" << std::endl;
	std::cout << "c: ��CTPģ����Բ���" << std::endl;
	std::cout << "***************************************************" << std::endl;
	std::cout << "��ѡ�������Ŀ��a, b, c):" << std::endl;
	std::cin >> ch;

	std::cin.get(); // erase the backspace.
	if (ch == 'a')
	{
		char json_src_path[MAX_PATH]; char json_dest_path[MAX_PATH];
		std::cout << "����δ��ʽ��JSON�ļ�·��" << std::endl;
		std::cin.getline(json_src_path, MAX_PATH);
		std::cout << "�����ʽ��JSON�ļ�Ŀ��·��" << std::endl;
		std::cin.getline(json_dest_path, MAX_PATH);

		Util::FormatJsonFile(json_src_path, json_dest_path);
	}
	else if (ch == 'b')
	{
		InstrumentIdAndHistoryDataPath instrumentIdData;
		InputInstrumentIdPath(instrumentIdData);

		const std::string& instrumentId = instrumentIdData.first;
		const std::string& instrumentDataPath = instrumentIdData.second;

		Json::CharReaderBuilder builder = Json::CharReaderBuilder();
		builder["collectComments"] = false;
		Json::Value value;

		std::ifstream config_doc(instrumentDataPath, std::ifstream::binary);

		JSONCPP_STRING errs;
		bool ok = Json::parseFromStream(builder, config_doc, &value, &errs);
		assert(ok && "������ʷ���ݴ���!!!");
		//config_doc.close();

		if (value.size() < 10)
		{
			std::cout << "��ʷ����̫���޷�����MA10������" << std::endl;
			return 1;
		}

		std::vector<MAData> history_MA; history_MA.reserve(value.size());

		TransactionManager transactionManager;

		// �ӵ�11�쿪ʼ�����������MA5/MA10
		for (int i = 10; i < (int)value.size(); ++i)
		{
			double ma_10 = 0.0;
			for (int j = i - 10; j < i; j++)
			{
				ma_10 += ::atof(value[j]["close"].asCString());
			}
			ma_10 = (ma_10 / 10);

			double ma_5 = 0.0;
			for (int k = i - 5; k < i; k++)
			{
				ma_5 += ::atof(value[k]["close"].asCString());
			}
			ma_5 = (ma_5 / 5);

			MAData maData;
			maData.Date = value[i - 1]["date"].asString();
			maData.MA5 = ma_5;
			maData.MA10 = ma_10;
			history_MA.push_back(std::move(maData));

			const size_t length = history_MA.size();
			// С�������MA��ʷ�����޷�ȷ�����ƣ�����������һ���MA5/MA10
			if (length < 2)
				continue;

			// ����MA5/10��ȥ���������
			const bool bMA5_Up = (history_MA[length - 1].MA5 - history_MA[length - 2].MA5) > 0;
			const bool bMA10_Up = (history_MA[length - 1].MA10 - history_MA[length - 2].MA10) > 0;

			// �����ȥ����MA5/10��λ�ù�ϵ
			const bool bMA5GreaterThanMA10_Yesterday = (history_MA[length - 1].MA5 - history_MA[length - 1].MA10) > 0;
			const bool bMA5GreaterThanMA10_Before_Yesterday = (history_MA[length - 2].MA5 - history_MA[length - 2].MA10) > 0;

			// ��ȥ����MA5��MA10�����������ƣ� ��MA5��������ͻ��MA10
			if (bMA5_Up && bMA10_Up && !bMA5GreaterThanMA10_Before_Yesterday && bMA5GreaterThanMA10_Yesterday)
			{
				// �Խ��쿪�̼�ƽ���ղ֣�����еĻ�
				transactionManager.ClosePosition(instrumentId, value[i]["date"].asString(), Position::eSell, ::atof(value[i]["open"].asCString()));

				// Ȼ�����Խ��쿪�̼ۿ����
				transactionManager.OpenPosition(instrumentId, value[i]["date"].asString(), Position::eBuy, ::atof(value[i]["open"].asCString()), 1);
			}
			// ��ȥ����MA5��MA10�������£� ��MA5�������µ���MA10
			else if (!bMA5_Up && !bMA10_Up && bMA5GreaterThanMA10_Before_Yesterday && !bMA5GreaterThanMA10_Yesterday)
			{
				//���Խ��쿪�̼�ƽ����֣�����еĻ�
				transactionManager.ClosePosition(instrumentId, value[i]["date"].asString(), Position::eBuy, ::atof(value[i]["open"].asCString()));

				// Ȼ�����Խ��쿪�̼ۿ��ղ�
				transactionManager.OpenPosition(instrumentId, value[i]["date"].asString(), Position::eSell, ::atof(value[i]["open"].asCString()), 1);
			}
			// ��ȥ����MA5�������µ���MA10
			else if (bMA5GreaterThanMA10_Before_Yesterday && !bMA5GreaterThanMA10_Yesterday)
			{
				// �Խ��쿪�̼�ƽ����֣�������еĻ�
				transactionManager.ClosePosition(instrumentId, value[i]["date"].asString(), Position::eBuy, ::atof(value[i]["open"].asCString()));
			}
			// ��ȥ����MA5���������ƣ���MA10�������ƣ���MA5��������ͻ��MA10
			else if (bMA5_Up && !bMA10_Up && !bMA5GreaterThanMA10_Before_Yesterday && bMA5GreaterThanMA10_Yesterday)
			{
				// �Խ��쿪�̼�ƽ���ղ֣�������еĻ�
				transactionManager.ClosePosition(instrumentId, value[i]["date"].asString(), Position::eSell, ::atof(value[i]["open"].asCString()));
			}
			else
			{
				// ����������������ֲ֣�����ȡ�κζ���
			}
		}

		// ���� MA5/10 ���ݵ��ļ�
		std::ofstream MADataFile;
		std::string maFilePath = "c:\\temp\\" + instrumentId + "_MAData.csv";

		MADataFile.open(maFilePath);
		MADataFile << "Date, " << "MA5, " << "MA10" << std::endl;
		for (const auto& ma : history_MA)
		{
			MADataFile << ma.Date << ","
				<< ma.MA5 << ","
				<< ma.MA10 << std::endl;
		}
		MADataFile.close();

		transactionManager.DumpCurrentStatus();
	}
	else if (ch == 'c')
	{
		CThostFtdcTraderApi *pUserTradeApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
		assert(pUserTradeApi != nullptr && "Create trader api failed");
		if (pUserTradeApi)
		{
			QCTPTradingSpi* pUserTradeSpi = new QCTPTradingSpi(pUserTradeApi);
			assert(pUserTradeSpi);
			if (pUserTradeSpi)
			{
				pUserTradeApi->RegisterSpi(pUserTradeSpi);
				pUserTradeApi->RegisterFront(Trade_Front);

				pUserTradeApi->SubscribePublicTopic(THOST_TERT_RESTART);
				pUserTradeApi->SubscribePrivateTopic(THOST_TERT_RESUME);

				pUserTradeApi->Init();
				pUserTradeApi->Join();

				pUserTradeApi->Release();

				delete pUserTradeSpi;
				pUserTradeSpi = nullptr;

			}
		}
	}

	std::cout << "���Խ���" << std::endl;
	std::cin.get();

    return 0;
}

