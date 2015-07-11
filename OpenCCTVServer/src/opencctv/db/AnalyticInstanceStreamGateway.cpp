/*
 * AnalyticInstanceStreamGateway.cpp
 *
 *  Created on: Jun 3, 2015
 *      Author: anjana
 */

#include "AnalyticInstanceStreamGateway.hpp"

namespace opencctv {
namespace db {

const std::string AnalyticInstanceStreamGateway::_SELECT_ALL_FOR_STREAM_SQL = "SELECT ainsts.id, ainsts.analytic_instance_id, ainps.name, a.filename FROM analytic_instance_streams AS ainsts, analytic_input_streams AS ainps, analytics AS a WHERE (ainsts.stream_id = ?) AND (ainsts.analytic_input_stream_id = ainps.id ) AND (ainps.analytic_id = a.id)";
const std::string AnalyticInstanceStreamGateway::_SELECT_ALL_FOR_ANALYTIC_INSTANCE_SQL = "";

AnalyticInstanceStreamGateway::AnalyticInstanceStreamGateway()
{
	try
	{
		_pDbConnectorPtr = new DbConnector();
		_pConnectionPtr = (*_pDbConnectorPtr).getConnection();

	}catch(sql::SQLException &e)
	{
		std::string sErrorMsg = "Error while initializing the AnalyticInstanceStreamGateway - .";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
	}
	catch(opencctv::Exception& e)
	{
		throw opencctv::Exception(e);
	}

	_pStatementPtr = NULL;
}

void AnalyticInstanceStreamGateway::findAllForStream(unsigned int iStreamId, std::vector<opencctv::dto::AnalyticInstanceStream>& vToStoreAIS)
{
	try
	{
		_pStatementPtr = (*_pConnectionPtr).prepareStatement(_SELECT_ALL_FOR_STREAM_SQL);
		(*_pStatementPtr).setInt(1, iStreamId);
		sql::ResultSet* pResultsPtr = (*_pStatementPtr).executeQuery();

		opencctv::dto::AnalyticInstanceStream ais;
		while((*pResultsPtr).next())
		{
			ais.setId((*pResultsPtr).getInt("id"));
			ais.setAnalyticInstanceId((*pResultsPtr).getInt("analytic_instance_id"));
			ais.setInputName((*pResultsPtr).getString("name"));
			//ais.setAnalyticFilename((*pResultsPtr).getString("filename"));
			ais.setAnalyticDirLocation((*pResultsPtr).getString("filename"));

			vToStoreAIS.push_back(ais);
			//ainsts.id, ainsts.analytic_instance_id, ainsts.analytic_input_stream_id, ainps.name, a.filename
		}
		(*pResultsPtr).close();
		delete pResultsPtr;
	}
	catch(sql::SQLException &e)
	{
		//std::cerr << "AnalyticGateway:findAnalytics: Error while finding analytics from the database. " << e.what() << std::endl;
		std::string sErrorMsg = "AnalyticInstanceStreamGateway::findAllForStream: ";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
		//TODO :: Add to log
	}
}

void AnalyticInstanceStreamGateway::findAllForAnalyticInstance(unsigned int iAnalyticInstanceId, std::vector<opencctv::dto::AnalyticInstanceStream>& vAnalyticInstanceStream)
{

}

AnalyticInstanceStreamGateway::~AnalyticInstanceStreamGateway()
{
	(*_pStatementPtr).close();
	delete _pStatementPtr;  _pStatementPtr = NULL;
	delete _pConnectionPtr; _pConnectionPtr = NULL;
	delete _pDbConnectorPtr; _pDbConnectorPtr = NULL;
}

} /* namespace db */
} /* namespace opencctv */


