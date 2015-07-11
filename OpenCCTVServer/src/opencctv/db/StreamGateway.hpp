/*
 * StreamGateway.hpp
 *
 *  Created on: Jun 3, 2015
 *      Author: anjana
 */

#ifndef STREAMGATEWAY_HPP_
#define STREAMGATEWAY_HPP_

#include <vector>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "DbConnector.hpp"
#include "../dto/Stream.hpp"
#include "../Exception.hpp"
#include "../util/Config.hpp"
#include "../util/Util.hpp"

namespace opencctv {
namespace db {

class StreamGateway {
private:
	DbConnector* _pDbConnectorPtr;
	sql::Connection* _pConnectionPtr;
	static const std::string _SELECT_ALL_STREAMS_SQL;
	static const std::string _SELECT_STREAM_SQL;

public:
	StreamGateway();
	virtual ~StreamGateway();
	//std::vector<opencctv::dto::Stream> findAll();
	void findAll(std::vector<opencctv::dto::Stream>& vToStoreStreams);
	void findStream(const unsigned int iStreamId, opencctv::dto::Stream& stream);
};

} /* namespace db */
} /* namespace opencctv */

#endif /* STREAMGATEWAY_HPP_ */
