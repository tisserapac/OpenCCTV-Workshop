/*
 * StreamGateway.cpp
 *
 *  Created on: Jun 3, 2015
 *      Author: anjana
 */

#include "StreamGateway.hpp"

namespace opencctv {
namespace db {

// TODO : Checking the verification field value of the streams are temporarily removed from the SQL queries.
/*
const std::string StreamGateway::_SELECT_ALL_STREAMS_SQL = "SELECT DISTINCT s.id, s.width, s.height, s.keep_aspect_ratio, s.allow_upsizing, s.compression_rate, ca.camera_id, vms.vms_connector_id, vms.server_ip, vms.server_port, vms.username, vms.password, vms.camera_url, vmsc.filename FROM streams AS s, cameras AS ca, vmses as vms, vms_connectors as vmsc WHERE (s.verified = TRUE) AND (s.camera_id = ca.id) AND (ca.vms_id = vms.id) AND (vms.vms_connector_id = vmsc.id) AND (s.id IN (SELECT DISTINCT ais.stream_id FROM analytic_instance_streams as ais))";
const std::string StreamGateway::_SELECT_STREAM_SQL ="SELECT DISTINCT s.id, s.width, s.height, s.keep_aspect_ratio, s.allow_upsizing, s.compression_rate, ca.camera_id, vms.vms_connector_id, vms.server_ip, vms.server_port, vms.username, vms.password, vms.camera_url, vmsc.filename FROM streams AS s, cameras AS ca, vmses as vms, vms_connectors as vmsc WHERE (s.verified = TRUE) AND (s.camera_id = ca.id) AND (ca.vms_id = vms.id) AND (vms.vms_connector_id = vmsc.id) AND s.id = ?";
*/
const std::string StreamGateway::_SELECT_ALL_STREAMS_SQL = "SELECT DISTINCT s.id, s.width, s.height, s.keep_aspect_ratio, s.allow_upsizing, s.compression_rate, ca.camera_id, vms.vms_connector_id, vms.server_ip, vms.server_port, vms.username, vms.password, vms.camera_url, vmsc.filename FROM streams AS s, cameras AS ca, vmses as vms, vms_connectors as vmsc WHERE (s.camera_id = ca.id) AND (ca.vms_id = vms.id) AND (vms.vms_connector_id = vmsc.id) AND (s.id IN (SELECT DISTINCT ais.stream_id FROM analytic_instance_streams as ais))";
const std::string StreamGateway::_SELECT_STREAM_SQL ="SELECT DISTINCT s.id, s.width, s.height, s.keep_aspect_ratio, s.allow_upsizing, s.compression_rate, ca.camera_id, vms.vms_connector_id, vms.server_ip, vms.server_port, vms.username, vms.password, vms.camera_url, vmsc.filename FROM streams AS s, cameras AS ca, vmses as vms, vms_connectors as vmsc WHERE (s.camera_id = ca.id) AND (ca.vms_id = vms.id) AND (vms.vms_connector_id = vmsc.id) AND s.id = ?";

StreamGateway::StreamGateway()
{
	try
	{
		_pDbConnectorPtr = new DbConnector();
		_pConnectionPtr = (*_pDbConnectorPtr).getConnection();
	}catch(sql::SQLException &e)
	{
		std::string sErrorMsg = "Error while initializing the StreamGateway - .";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
	}
	catch(opencctv::Exception& e)
	{
		throw opencctv::Exception(e);
	}

}

void StreamGateway::findStream(const unsigned int iStreamId, opencctv::dto::Stream& stream)
{
	try
	{
		sql::PreparedStatement* pStatementPtr;
		pStatementPtr = (*_pConnectionPtr).prepareStatement(_SELECT_STREAM_SQL);
		(*pStatementPtr).setInt(1, iStreamId);
		sql::ResultSet* pResultsPtr = (*pStatementPtr).executeQuery();
		while((*pResultsPtr).next())
		{
			stream.setId((*pResultsPtr).getInt("id"));
			stream.setWidth((*pResultsPtr).getInt("width"));
			stream.setHeight((*pResultsPtr).getInt("height"));
			stream.setKeepAspectRatio((*pResultsPtr).getBoolean("keep_aspect_ratio"));
			stream.setAllowUpSizing((*pResultsPtr).getBoolean("allow_upsizing"));
			stream.setCompressionRate((*pResultsPtr).getInt("compression_rate"));
			stream.setCameraId((*pResultsPtr).getString("camera_id"));
			stream.setVmsTypeId((*pResultsPtr).getInt("vms_connector_id"));
			stream.setVmsServerIp((*pResultsPtr).getString("server_ip"));
			stream.setVmsServerPort((*pResultsPtr).getInt("server_port"));
			stream.setVmsUsername((*pResultsPtr).getString("username"));
			stream.setVmsPassword((*pResultsPtr).getString("password"));

			/* This field is only used by the direct camera connections */
			stream.setCameraUri((*pResultsPtr).getString("camera_url"));

			stream.setVmsConnectorFilename((*pResultsPtr).getString("filename"));
			stream.setVmsConnectorDirLocation((*pResultsPtr).getString("filename"));
		}
		(*pResultsPtr).close();
		(*pStatementPtr).close();
		delete pResultsPtr; pResultsPtr = NULL;
		delete pStatementPtr; pStatementPtr = NULL;

	}catch(sql::SQLException &e)
	{
		std::string sErrorMsg = "StreamGateway:findStream : ";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
		// TODO: log
	}
}

void StreamGateway::findAll(std::vector<opencctv::dto::Stream>& vToStoreStreams)
{
	try
	{
		sql::Statement* pStatement = (*_pConnectionPtr).createStatement();
		sql::ResultSet* pResultsPtr = (*pStatement).executeQuery(_SELECT_ALL_STREAMS_SQL);
		opencctv::dto::Stream stream;
		while((*pResultsPtr).next())
		{
			stream.setId((*pResultsPtr).getInt("id"));
			stream.setWidth((*pResultsPtr).getInt("width"));
			stream.setHeight((*pResultsPtr).getInt("height"));
			stream.setKeepAspectRatio((*pResultsPtr).getBoolean("keep_aspect_ratio"));
			stream.setAllowUpSizing((*pResultsPtr).getBoolean("allow_upsizing"));
			stream.setCompressionRate((*pResultsPtr).getInt("compression_rate"));
			stream.setCameraId((*pResultsPtr).getString("camera_id"));
			stream.setVmsTypeId((*pResultsPtr).getInt("vms_connector_id"));
			stream.setVmsServerIp((*pResultsPtr).getString("server_ip"));
			stream.setVmsServerPort((*pResultsPtr).getInt("server_port"));
			stream.setVmsUsername((*pResultsPtr).getString("username"));
			stream.setVmsPassword((*pResultsPtr).getString("password"));

			/* This field is only used by the direct camera connections */
			stream.setCameraUri((*pResultsPtr).getString("camera_url"));

			stream.setVmsConnectorFilename((*pResultsPtr).getString("filename"));
			stream.setVmsConnectorDirLocation((*pResultsPtr).getString("filename"));

			vToStoreStreams.push_back(stream);
		}
		(*pResultsPtr).close();
		(*pStatement).close();
		delete pResultsPtr; pResultsPtr = NULL;
		delete pStatement; pStatement = NULL;

	}catch(sql::SQLException &e)
	{
		std::string sErrorMsg = "StreamGateway:findAll: ";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
		//std::cerr << "StreamGateway:findAll: Error while finding all streams from the database. " << e.what() << std::endl;
		// TODO: log
	}
}

StreamGateway::~StreamGateway()
{
	/*(*_pStatement).close();
	delete _pStatement; _pStatement = NULL;*/
	delete _pConnectionPtr; _pConnectionPtr = NULL;
	delete _pDbConnectorPtr; _pDbConnectorPtr = NULL;
}

} /* namespace db */
} /* namespace opencctv */
