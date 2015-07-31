/*
 * AnalyticInstance.cpp
 *
 *  Created on: Jul 9, 2015
 *      Author: anjana
 */

#include "AnalyticInstance.hpp"

namespace opencctv {
namespace dto {

AnalyticInstance::AnalyticInstance()
{
	_iId = 0;
	_iAnalyticId = 0;
}

AnalyticInstance::~AnalyticInstance()
{
}

unsigned int AnalyticInstance::getAnalyticId() const
{
	return _iAnalyticId;
}

void AnalyticInstance::setAnalyticId(unsigned int analyticId)
{
	_iAnalyticId = analyticId;
}

unsigned int AnalyticInstance::getId() const
{
	return _iId;
}

void AnalyticInstance::setId(unsigned int id)
{
	_iId = id;
}

const std::string& AnalyticInstance::getDescription() const
{
	return _sDescription;
}

void AnalyticInstance::setDescription(const std::string& description)
{
	_sDescription = description;
}


const std::string& AnalyticInstance::getAnalyticDirLocation() const {
	return _sAnalyticDirLocation;
}

void AnalyticInstance::setAnalyticDirLocation(const std::string& analyticDirLocation) {
	_sAnalyticDirLocation = analyticDirLocation;
}

const std::string& AnalyticInstance::getName() const
{
	return _sName;
}

void AnalyticInstance::setName(const std::string& name)
{
	_sName = name;
}

} /* namespace dto */
} /* namespace opencctv */
