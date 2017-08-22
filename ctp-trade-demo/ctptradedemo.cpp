// ctptradedemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../tradeapi_x64/ThostFtdcTraderApi.h"

#include "QCTPTradingSpi.h"

// 7x24:       180.168.146.187:10030
// standard:   180.168.146.187:10001
char* Trade_Front = "tcp://180.168.146.187:10001";

int main()
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

	

    return 0;
}

