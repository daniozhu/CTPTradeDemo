#pragma once
#include "..\tradeapi_x64\ThostFtdcTraderApi.h"

class QCTPTradingApi :
	public CThostFtdcTraderSpi
{
public:
	QCTPTradingApi(CThostFtdcTraderApi* pTradeApi);
	virtual ~QCTPTradingApi();

	virtual void OnFrontConnected() override;
	virtual void OnFrontDisconnected(int nReason) override;
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

private:
	CThostFtdcTraderApi*	m_pUserTradeApi;
};
