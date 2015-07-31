/*
 * AnalyticInstance.hpp
 *
 *  Created on: Jul 9, 2015
 *      Author: anjana
 */

#ifndef ANALYTICINSTANCE_HPP_
#define ANALYTICINSTANCE_HPP_

#include <string>

namespace opencctv {
namespace dto {

class AnalyticInstance {
private:
	unsigned int _iId;
	unsigned int _iAnalyticId;
	std::string _sName;
	std::string _sDescription;
	std::string _sAnalyticDirLocation;

public:
	AnalyticInstance();
	virtual ~AnalyticInstance();

	unsigned int getAnalyticId() const;
	void setAnalyticId(unsigned int analyticId);
	unsigned int getId() const;
	void setId(unsigned int id);
	const std::string& getDescription() const;
	void setDescription(const std::string& description);
	const std::string& getName() const;
	void setName(const std::string& name);
	const std::string& getAnalyticDirLocation() const;
	void setAnalyticDirLocation(const std::string& analyticDirLocation);
};

} /* namespace dto */
} /* namespace opencctv */

#endif /* ANALYTICINSTANCE_HPP_ */
