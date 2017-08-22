#include "stdafx.h"
#include "QCTPTradingSpi.h"

const char* BrokerID  = "9999";
const char* UserID    = "082644";
const char* Password  = "123456789";

QCTPTradingSpi::QCTPTradingSpi(CThostFtdcTraderApi * pTradeApi)
	: m_pUserTradeApi(pTradeApi)
{
	assert(m_pUserTradeApi);
}


QCTPTradingSpi::~QCTPTradingSpi()
{
	m_pUserTradeApi = nullptr;
}

void QCTPTradingSpi::OnFrontConnected()
{
	std::cout << "OnFrontConnected" << std::endl;
	if (m_pUserTradeApi)
	{
		CThostFtdcReqUserLoginField loginField;
		memset(&loginField, 0, sizeof(loginField));

		strcpy_s(loginField.BrokerID, BrokerID);
		strcpy_s(loginField.UserID, UserID);
		strcpy_s(loginField.Password, Password);

		static int RequstID = 0;
		int nRet = m_pUserTradeApi->ReqUserLogin(&loginField, RequstID++);

		std::cout << "Send login request " << (0 == nRet ? "Succeeded" : "Failed") << std::endl;
	}
}

void QCTPTradingSpi::OnFrontDisconnected(int nReason)
{
	std::cout << "OnFrontDisconnected due to reason: " << nReason << std::endl;
}

void QCTPTradingSpi::OnRspUserLogin(CThostFtdcRspUserLoginField * pRspUserLogin, CThostFtdcRspInfoField * pRspInfo, int nRequestID, bool bIsLast)
{
	const bool bOK = (pRspInfo && pRspInfo->ErrorID == 0);
	std::cout << "OnRspUserLogin " << (bOK ? "Succeeded" : "Failed") << std::endl;
}

void QCTPTradingSpi::OnRspError(CThostFtdcRspInfoField * pRspInfo, int nRequestID, bool bIsLast)
{
	std::cout << "OnRepError - ErrorId = " << pRspInfo->ErrorID << std::endl;
}

