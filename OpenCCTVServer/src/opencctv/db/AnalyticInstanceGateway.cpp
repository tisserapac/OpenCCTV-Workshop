/*
 * AnalyticInstanceGateway.cpp
 *
 *  Created on: Jul 9, 2015
 *      Author: anjana
 */

#include "AnalyticInstanceGateway.hpp"

namespace opencctv {
namespace db {

const std::string AnalyticInstanceGateway::_SELECT_ANALYTIC_INSTANCE_SQL = "SELECT ai.id, ai.name, ai.description, ai.analytic_id, a.filename FROM analytic_instances as ai, analytics a WHERE ai.analytic_id = a.id AND ai.id = ?";
const std::string AnalyticInstanceGateway::_UPDATE_ALL_SQL = "UPDATE analytic_instances SET status = ? WHERE id > 0";
const std::string AnalyticInstanceGateway::_UPDATE_ANALYTIC_INSTANCE_SQL = "UPDATE analytic_instances SET status = ? WHERE id = ?";
const std::string AnalyticInstanceGateway::_SELECT_ANALYTIC_INSTANCE_STATUS_SQL = "SELECT status from analytic_instances WHERE id = ?";

AnalyticInstanceGateway::AnalyticInstanceGateway()
{
	try
	{
		_pDbConnectorPtr = new DbConnector();
		_pConnectionPtr = (*_pDbConnectorPtr).getConnection();


	}catch(sql::SQLException &e)
	{
		std::string sErrorMsg = "Error while initializing the AnalyticInstanceGateway - .";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
	}
	catch(opencctv::Exception& e)
	{
		throw opencctv::Exception(e);
	}

}

void AnalyticInstanceGateway::updateAllAnalyticInstanceStatus()
{
	sql::PreparedStatement* _pStatementPtr = NULL;
	std::string sStatus =  opencctv::ApplicationModel::getInstance()->getServerStatus();

	try
	{

		_pStatementPtr = (*_pConnectionPtr).prepareStatement(_UPDATE_ALL_SQL);
		(*_pStatementPtr).setString(1, sStatus);
		(*_pStatementPtr).executeUpdate();

		(*_pStatementPtr).close();
		delete _pStatementPtr;  _pStatementPtr = NULL;
	}
	catch(sql::SQLException &e)
	{
		(*_pStatementPtr).close();
		delete _pStatementPtr;  _pStatementPtr = NULL;

		std::string sErrorMsg = "AnalyticInstanceGateway::updateAllAnalyticStatus : ";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
		//TODO :: Add to log
	}

}

void AnalyticInstanceGateway::updateAnalyticInstanceStatus(const unsigned int iAnalyticInstanceId, const std::string& sStatus)
{
	sql::PreparedStatement* _pStatementPtr = NULL;

	try
	{
		_pStatementPtr = (*_pConnectionPtr).prepareStatement(_UPDATE_ANALYTIC_INSTANCE_SQL);
		(*_pStatementPtr).setString(1, sStatus);
		(*_pStatementPtr).setInt(2, iAnalyticInstanceId);
		(*_pStatementPtr).executeUpdate();

		(*_pStatementPtr).close();
		delete _pStatementPtr;  _pStatementPtr = NULL;
	}
	catch(sql::SQLException &e)
	{
		(*_pStatementPtr).close();
		delete _pStatementPtr;  _pStatementPtr = NULL;

		std::string sErrorMsg = "AnalyticInstanceGateway::updateAllAnalyticStatus : ";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
		//TODO :: Add to log
	}

}

std::string AnalyticInstanceGateway::getAnalyticInstanceStatus(const unsigned int iAnalyticInstanceId)
{
	sql::PreparedStatement* _pStatementPtr = NULL;
	sql::ResultSet* pResultsPtr = NULL;
	std::string sStatus = "";
	try
	{

		_pStatementPtr = (*_pConnectionPtr).prepareStatement(_SELECT_ANALYTIC_INSTANCE_STATUS_SQL);
		(*_pStatementPtr).setInt(1, iAnalyticInstanceId);
		pResultsPtr = (*_pStatementPtr).executeQuery();

		while((*pResultsPtr).next())
		{
			sStatus = (*pResultsPtr).getString("status");
		}
		(*pResultsPtr).close();
		delete pResultsPtr; pResultsPtr = NULL;
		(*_pStatementPtr).close();
		delete _pStatementPtr;  _pStatementPtr = NULL;
	}
	catch(sql::SQLException &e)
	{
		(*pResultsPtr).close();
		delete pResultsPtr; pResultsPtr = NULL;
		(*_pStatementPtr).close();
		delete _pStatementPtr;  _pStatementPtr = NULL;

		std::string sErrorMsg = "AnalyticInstanceGateway::getAnalyticInstanceStatus: ";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
		//TODO :: Add to log
	}

	return sStatus;
}

void AnalyticInstanceGateway::findAnalyticInstance(const unsigned int iAnalyticInstanceId, opencctv::dto::AnalyticInstance& ai)
{
	sql::PreparedStatement* _pStatementPtr = NULL;
	sql::ResultSet* pResultsPtr = NULL;
	try
	{

		_pStatementPtr = (*_pConnectionPtr).prepareStatement(_SELECT_ANALYTIC_INSTANCE_SQL);
		(*_pStatementPtr).setInt(1, iAnalyticInstanceId);
		pResultsPtr = (*_pStatementPtr).executeQuery();

		while((*pResultsPtr).next())
		{
			//id, name, description, analytic_id, filename
			ai.setId((*pResultsPtr).getInt("id"));
			ai.setName((*pResultsPtr).getString("name"));
			ai.setDescription((*pResultsPtr).getString("description"));
			ai.setAnalyticId((*pResultsPtr).getInt("analytic_id"));
			ai.setAnalyticDirLocation((*pResultsPtr).getString("filename"));
		}
		(*pResultsPtr).close();
		delete pResultsPtr; pResultsPtr = NULL;
		(*_pStatementPtr).close();
		delete _pStatementPtr;  _pStatementPtr = NULL;
	}
	catch(sql::SQLException &e)
	{
		(*pResultsPtr).close();
		delete pResultsPtr; pResultsPtr = NULL;
		(*_pStatementPtr).close();
		delete _pStatementPtr;  _pStatementPtr = NULL;

		std::string sErrorMsg = "AnalyticInstanceGateway::findAnalyticInstance: ";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
		//TODO :: Add to log
	}
}

AnalyticInstanceGateway::~AnalyticInstanceGateway()
{
	delete _pConnectionPtr; _pConnectionPtr = NULL;
	delete _pDbConnectorPtr; _pDbConnectorPtr = NULL;
}

} /* namespace db */
} /* namespace opencctv */
