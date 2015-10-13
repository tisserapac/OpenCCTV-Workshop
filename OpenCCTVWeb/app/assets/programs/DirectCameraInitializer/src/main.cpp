/*
 * main.cpp
 *
 *  Created on: Oct 13, 2015
 *      Author: anjana
 */

#include <sstream>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

/*
#include <stdio.h>
#include <stdlib.h>
*/

std::string generateOutput(bool bVerified, const std::string& sMessage, const int iWidth, const int iHeight)
{
	std::stringstream ss;
	ss << "<camerainitializerreply>";
		ss << "<verified>" << bVerified << "</verified>";
		ss << "<message>" << sMessage << "</message>";
		ss << "<width>" << iWidth << "</width>";
		ss << "<height>" << iHeight << "</height>";
	ss << "</camerainitializerreply>";
	return ss.str();
}

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		return -1;
	}

	std::string sUrl = argv[1];
	std::string sPathToImageFile = argv[2];

	cv::VideoCapture vcap;
	//cv::VideoCapture vcap(0);
	cv::Mat image;
	std::string sReplyMsg = "";

	bool bResult = false;
	std::string sStremaType = "http://";

	//std::cout<< "sUrl : " << sUrl << std::endl;

	bResult = vcap.open(sUrl);
	//bResult = vcap.isOpened();

	//If capture fails and the stream is http, append "dummy=param.mjpg" to the URL and try again
	if(!bResult && (sUrl.compare(0,sStremaType.length(), sStremaType) == 0))
	{
		std::string sNewUrl = sUrl;

		if (*sUrl.rbegin() == '/') // Remove '/' if it is the last char
		{
			sNewUrl = sUrl.substr(0, sUrl.size()-1);
		}

		sNewUrl.append("?dummy=param.mjpg");
		//std::cout<< "sNewUrl : " << sNewUrl << std::endl;
		bResult = vcap.open(sNewUrl);
	}

	if(!bResult)//If the capturing is still failing, return as a validation failure
	{
		sReplyMsg = generateOutput(bResult, "Camera verification failed", 0, 0);
	}
	else //If successful try to read an image and save it to the given path
	{
		int i = 0;
		bResult = false;

		while(i<20)
		{
			bResult = vcap.read(image);
			if(bResult)
			{
				if(cv::imwrite(sPathToImageFile, image))
				{
					sReplyMsg = generateOutput(true, "Camera verification successful", vcap.get(CV_CAP_PROP_FRAME_WIDTH), vcap.get(CV_CAP_PROP_FRAME_HEIGHT));
				}else
				{
					sReplyMsg = generateOutput(true, "Failed to save the default frame from the camera", vcap.get(CV_CAP_PROP_FRAME_WIDTH), vcap.get(CV_CAP_PROP_FRAME_HEIGHT));
				}

				break;
			}
			++i;
		}

		if(!bResult)//If fail to capture an image after 20 tries, return as a validation failure
		{
			sReplyMsg = generateOutput(false, "Failed to capture the default image from the camera ", 0, 0);
		}
	}

	std::cout << sReplyMsg << std::endl;
	std::cout.flush();

	return 0;
}


