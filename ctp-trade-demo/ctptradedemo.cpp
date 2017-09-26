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

//注意：　从新浪财经下载的数据不是格式化的JSON数据， JsonCPP无法识别，需要先将所有的Key都添加引号。
//http://stock.finance.sina.com.cn/futures/api/json.php/InnerFuturesService.getInnerFuturesDailyKLine?symbol=RB1710

using InstrumentIdAndHistoryDataPath = std::pair<std::string, std::string>;

namespace
{
	void InputInstrumentIdPath(InstrumentIdAndHistoryDataPath& instrumentIdData)
	{
		std::cout << "输入合约编号：" << std::endl;
		char instrumentId[MAX_PATH];
		std::cin.getline(instrumentId, MAX_PATH);

		std::cout << "输入历史数据路径：" << std::endl;
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
	std::cout << "a: 格式化历史数据（JSON） 文件" << std::endl;
	std::cout << "b: 用历史数据回测策略" << std::endl;
	std::cout << "c: 用CTP模拟测试策略" << std::endl;
	std::cout << "***************************************************" << std::endl;
	std::cout << "请选择操作项目（a, b, c):" << std::endl;
	std::cin >> ch;

	std::cin.get(); // erase the backspace.
	if (ch == 'a')
	{
		char json_src_path[MAX_PATH]; char json_dest_path[MAX_PATH];
		std::cout << "输入未格式化JSON文件路径" << std::endl;
		std::cin.getline(json_src_path, MAX_PATH);
		std::cout << "输入格式化JSON文件目标路径" << std::endl;
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
		assert(ok && "解析历史数据错误!!!");
		//config_doc.close();

		if (value.size() < 10)
		{
			std::cout << "历史数据太少无法计算MA10！！！" << std::endl;
			return 1;
		}

		std::vector<MAData> history_MA; history_MA.reserve(value.size());

		TransactionManager transactionManager;

		std::ofstream MADataFile;
		std::string maFilePath = "c:\\temp\\" + instrumentId + "_MAData.csv";
		MADataFile.open(maFilePath);
		MADataFile << "Date, " << "MA5, " << "MA10" << std::endl;

		// 从第11天开始，计算昨天的MA5/MA10
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

			const size_t length = history_MA.size();

			MAData maData;
			maData.Date = value[i - 1]["date"].asString();
			maData.MA5 = ma_5;
			maData.MA10 = ma_10;
			
			MADataFile << maData.Date << ","
				<< maData.MA5 << ","
				<< maData.MA10 << std::endl;

			// 小于两天的MA历史数据无法确定走势，继续计算下一天的MA5/MA10
			if (0 == length)
			{
				history_MA.push_back(std::move(maData));
				continue;
			}

			// 计算昨天和前天的MA5/10趋势
			const bool bMA5_Up = (maData.MA5 - history_MA[length - 1].MA5) > 0;
			const bool bMA10_Up = (maData.MA10 - history_MA[length - 1].MA10) > 0;

			// 计算昨天和前天MA5/10的位置关系
			const bool bMA5GreaterThanMA10_Yesterday = (maData.MA5 - maData.MA10) > 0;
			const bool bMA5GreaterThanMA10_Before_Yesterday = (history_MA[length - 1].MA5 - history_MA[length - 1].MA10) > 0;

			// 能否计算大前天的M5/M10走势
			const bool bCanCheckTwoDaysBack = (length > 2);

			std::string today     =  value[i]["date"].asString();
			double todyOpenPrice  = ::atof(value[i]["open"].asCString());

			// 前天到昨天MA5和MA10都是向上趋势， 且MA5从下向上突破MA10
			if (bMA5_Up && bMA10_Up && !bMA5GreaterThanMA10_Before_Yesterday && bMA5GreaterThanMA10_Yesterday)
			{
				// 以今天开盘价平掉空仓，如果有的话
				transactionManager.ClosePosition(instrumentId, today, Position::eSell, todyOpenPrice);

				// M10前天和大前天也是上升趋势, 上升趋势已明显，以今天开盘价开多仓
				if (bCanCheckTwoDaysBack )
				{
					if(history_MA[length - 1].IsMA10UptrendFromYesterday && history_MA[length - 2].IsMA10UptrendFromYesterday)
						transactionManager.OpenPosition(instrumentId, today, Position::eBuy, todyOpenPrice, 1);
				}
				else 
				{
					//如果历史数据少于计算大前天的M10走势，以今天开盘价开多仓
					transactionManager.OpenPosition(instrumentId, today, Position::eBuy, todyOpenPrice, 1);
				}
			}
			// 过去两天MA5和MA10趋势向下， 且MA5从上往下跌破MA10
			else if (!bMA5_Up && !bMA10_Up && bMA5GreaterThanMA10_Before_Yesterday && !bMA5GreaterThanMA10_Yesterday)
			{
				//　以今天开盘价平掉多仓，如果有的话
				transactionManager.ClosePosition(instrumentId, today, Position::eBuy, todyOpenPrice);

				// 以今天开盘价开空仓
				transactionManager.OpenPosition(instrumentId, today, Position::eSell, todyOpenPrice, 1);
				
			}
			// 过去两天MA5从上往下跌破MA10
			else if (bMA5GreaterThanMA10_Before_Yesterday && !bMA5GreaterThanMA10_Yesterday)
			{
				// 以今天开盘价平掉多仓，　如果有的话
				transactionManager.ClosePosition(instrumentId, today, Position::eBuy, todyOpenPrice);
			}
			// 过去两天MA5呈上升趋势，　MA10向下趋势，且MA5从下往上突破MA10
			else if (bMA5_Up && !bMA10_Up && !bMA5GreaterThanMA10_Before_Yesterday && bMA5GreaterThanMA10_Yesterday)
			{
				// 以今天开盘价平掉空仓，　如果有的话
				transactionManager.ClosePosition(instrumentId, today, Position::eSell, todyOpenPrice);
			}
			else
			{
				// 其他情况，　继续持仓，不采取任何动作
			}

			maData.IsCrossedWithYesterday = (!bMA5GreaterThanMA10_Before_Yesterday && bMA5GreaterThanMA10_Yesterday)||
				(bMA5GreaterThanMA10_Before_Yesterday && !bMA5GreaterThanMA10_Yesterday);
			maData.IsMA5UpTrendFromYesterday = bMA5_Up;
			maData.IsMA10UptrendFromYesterday = bMA10_Up;

			history_MA.push_back(std::move(maData));
			transactionManager.IncreaseHoldingDay();
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

	std::cout << "测试结束" << std::endl;
	std::cin.get();

    return 0;
}

