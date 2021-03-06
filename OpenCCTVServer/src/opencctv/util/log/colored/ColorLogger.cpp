
#include "ColorLogger.hpp"

namespace opencctv {
namespace util {
namespace log {

ColorLogger::ColorLogger() {
}

void ColorLogger::error(const std::string& sMessage)
{
	boost::mutex::scoped_lock lock(_mutex);
	//std::cout << "Error: " << sMessage << std::endl; // to the pipe
	std::cerr << "Error: " << "\033[31m" << sMessage << "\033[0m" << std::endl;
}

void ColorLogger::warn(const std::string& sMessage)
{
	boost::mutex::scoped_lock lock(_mutex);
	std::cerr << "Warning: " << "\033[33m" << sMessage << "\033[0m" << std::endl;
}

void ColorLogger::info(const std::string& sMessage)
{
	boost::mutex::scoped_lock lock(_mutex);
	std::cerr << "Info: " << "\033[34m" << sMessage << "\033[0m" << std::endl;
}

void ColorLogger::debug(const std::string& sMessage)
{
	boost::mutex::scoped_lock lock(_mutex);
	std::cerr << "Debug: " << "\033[36m" << sMessage << "\033[0m" << std::endl;
}

ColorLogger::~ColorLogger() {
}

} /* namespace log */
} /* namespace util */
} /* namespace opencctv */
