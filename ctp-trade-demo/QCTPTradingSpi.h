#pragma once
#include "..\tradeapi_x64\ThostFtdcTraderApi.h"

class QCTPTradingSpi :
	public CThostFtdcTraderSpi
{
public:
	QCTPTradingSpi(CThostFtdcTraderApi* pTradeApi);
	virtual ~QCTPTradingSpi();

	virtual void OnFrontConnected() override;
	virtual void OnFrontDisconnected(int nReason) override;
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;


private:
	CThostFtdcTraderApi*	m_pUserTradeApi;
};
