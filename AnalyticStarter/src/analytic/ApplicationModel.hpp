
#ifndef ANALYTIC_APPLICATIONMODEL_HPP_
#define ANALYTIC_APPLICATIONMODEL_HPP_

#include "AnalyticProcess.hpp"
#include <map>

namespace analytic {

class ApplicationModel {
private:
	static ApplicationModel* _pModel;
	ApplicationModel();
	std::map<unsigned int, analytic::AnalyticProcess*> _mAnalyticProcesses; // Analytic Instance ID as key
	std::vector<unsigned int> _vPorts; //Vector of available port numbers on the analytic server

public:
	static ApplicationModel* getInstance();
	bool containsAnalyticProcess(unsigned int iAnalyticInstanceId);
	std::map<unsigned int, analytic::AnalyticProcess*>& getAnalyticProcesses();
	std::vector<unsigned int>& getAnalyticPorts();
	virtual ~ApplicationModel();
};

} /* namespace analytic */

#endif /* ANALYTIC_APPLICATIONMODEL_HPP_ */
