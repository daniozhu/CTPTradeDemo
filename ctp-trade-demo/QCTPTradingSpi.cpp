#include "stdafx.h"
#include "QCTPTradingSpi.h"

const char* BrokerID  = "9999";
const char* UserID    = "082644";
const char* Password  = "123456789";

int s_RequstID = 0;

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
	if (m_pUserTradeApi)
	{
		CThostFtdcReqUserLoginField loginField;
		memset(&loginField, 0, sizeof(loginField));

		strcpy_s(loginField.BrokerID, BrokerID);
		strcpy_s(loginField.UserID, UserID);
		strcpy_s(loginField.Password, Password);

		int nRet = m_pUserTradeApi->ReqUserLogin(&loginField, s_RequstID++);
		if (nRet != 0) 
		{
			std::cout << "Failed to send login request." << std::endl;
		}
	}
}

void QCTPTradingSpi::OnFrontDisconnected(int nReason)
{
	std::cout << "OnFrontDisconnected due to reason: " << nReason << std::endl;
}

void QCTPTradingSpi::OnRspUserLogin(CThostFtdcRspUserLoginField * pRspUserLogin, CThostFtdcRspInfoField * pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID != 0)
	{
		std::cout << "OnRespUserLogin failed - " << pRspInfo->ErrorMsg << std::endl;
	}

	CThostFtdcQrySettlementInfoField settlementInfoField;
	memset(&settlementInfoField, 0, sizeof(settlementInfoField));
	strcpy_s(settlementInfoField.BrokerID, BrokerID);
	strcpy_s(settlementInfoField.InvestorID, UserID);
	strcpy_s(settlementInfoField.TradingDay, "20170825");

	int nRet = m_pUserTradeApi->ReqQrySettlementInfo(&settlementInfoField, s_RequstID++);
	if (nRet != 0)
	{
		std::cout << "Failed to send request to query settlement info." << std::endl;
	}
}

void QCTPTradingSpi::OnRspError(CThostFtdcRspInfoField * pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo)
	{
		std::cout << "OnRepError -  = " << pRspInfo->ErrorMsg << std::endl;
	}
}

void QCTPTradingSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField * pSettlementInfo, CThostFtdcRspInfoField * pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID != 0)
	{
		std::cout << "OnRspQrySettlementInfo failed - " << pRspInfo->ErrorMsg << std::endl;
	}

	if (pSettlementInfo)
	{
		std::cout << pSettlementInfo->Content << std::endl;

		if ( bIsLast)
		{
			/*std::cout << "TradingDay: " << pSettlementInfo->TradingDay << std::endl;
			std::cout << "BrokerID: " << pSettlementInfo->BrokerID << std::endl;
			std::cout << "InvestorID: " << pSettlementInfo->InvestorID << std::endl;
			std::cout << "SequenceNo: " << pSettlementInfo->SequenceNo << std::endl;
			std::cout << "SettlementID: " << pSettlementInfo->SettlementID << std::endl;
			std::cout << "TradingDay" << pSettlementInfo->TradingDay << std::endl;*/

			char ch = '0';
			while (ch != 'y')
			{
				std::cout << "Please confirm (y/n):" << std::endl;
				std::cin >> ch;
			}
			
			CThostFtdcSettlementInfoConfirmField confirmField;
			memset(&confirmField, 0, sizeof(confirmField));
			strcpy_s(confirmField.BrokerID, BrokerID);
			strcpy_s(confirmField.InvestorID, UserID);

			int nRet = m_pUserTradeApi->ReqSettlementInfoConfirm(&confirmField, s_RequstID++);
			if (nRet != 0)
			{
				std::cout << "Failed to send settlement information confirmation request." << std::endl;
			}
		}
	}
}

void QCTPTradingSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField * pSettlementInfoConfirm, CThostFtdcRspInfoField * pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspInfo && pRspInfo->ErrorID != 0)
	{
		std::cout << "OnRspSettlementInfoConfirm failed - " << pRspInfo->ErrorMsg << std::endl;
	}

	if (pSettlementInfoConfirm)
	{
		std::cout << "SettlementInfo is confirmed: " << std::endl;

		std::cout << "BrokerID: " << pSettlementInfoConfirm->BrokerID << std::endl;
		std::cout << "InvestorID: " << pSettlementInfoConfirm->InvestorID << std::endl;
		std::cout << "ConfirmDate: " << pSettlementInfoConfirm->ConfirmDate << std::endl;
		std::cout << "ConfirmTime: " << pSettlementInfoConfirm->ConfirmTime << std::endl;
	}
}

