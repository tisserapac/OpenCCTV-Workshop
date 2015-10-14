/*
 * DirectCameraConnector.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: anjana
 */

#include "DirectCameraConnector.hpp"

opencctv::ConcurrentQueue<opencctv::Image>* _pQueue = NULL;

DirectCameraConnector::DirectCameraConnector() : opencctv::api::VmsConnector()
{
	_pContext = (struct ctx*)malloc(sizeof(*_pContext));
	_bEnable = false;
	_pVlcInstance = NULL;
	_pVlcMedia = NULL;
	_pVlcMediaPlayer = NULL;
}

bool DirectCameraConnector::init(const opencctv::api::VmsConnectInfo& info, const std::string sPathToAnalyticDir)
{
	// init context
	_pContext->pImage = cvCreateImage(cvSize(info.iWidth, info.iHeight), IPL_DEPTH_8U, 4);
	_pContext->puchPixels = (unsigned char*)_pContext->pImage->imageData;

	//Initializing the _pVlcInstance according to the stream type
	//TODO: At the moment only rtsp and http streams are supported; Any more stream types to support?
	std::string sStremaType = "rtsp://";

	if(info.sCameraUri.compare(0,sStremaType.length(), sStremaType) == 0)
	{
		const char * const aChVlcArgs[] = {"--rtsp-tcp","--verbose=-1"};
		_pVlcInstance = libvlc_new(sizeof(aChVlcArgs) / sizeof(aChVlcArgs[0]), aChVlcArgs);
	}else
	{
		const char * const aChVlcArgs[] = {"--verbose=-1"};
		_pVlcInstance = libvlc_new(sizeof(aChVlcArgs) / sizeof(aChVlcArgs[0]), aChVlcArgs);
		//_pVlcInstance = libvlc_new(0,NULL);
	}

   	std::cout << "libvlc is created." << std::endl;
   	_pVlcMedia = libvlc_media_new_location(_pVlcInstance, info.sCameraUri.c_str());
	_pVlcMediaPlayer = libvlc_media_player_new_from_media(_pVlcMedia);

	//Test This
	//libvlc_media_release (_pVlcMedia);

	libvlc_video_set_callbacks(_pVlcMediaPlayer, lock, unlock, display, _pContext);
	libvlc_video_set_format(_pVlcMediaPlayer, "RV32", info.iWidth, info.iHeight, info.iWidth * 4);

	_bEnable = true;
	return _bEnable;
}

void DirectCameraConnector::produceImageObjects(opencctv::ConcurrentQueue<opencctv::Image>* pQueue)
{
	_pQueue = pQueue;
	libvlc_media_player_play(_pVlcMediaPlayer);
	while (1)
	{
		try
		{
			boost::this_thread::sleep_for(boost::chrono::seconds(10));
		} catch (const boost::thread_interrupted&) {
			std::cout << "- Thread interrupted. Exiting thread." << std::endl;
			break;
		}
	}

	//Stop playing
	libvlc_media_player_stop (_pVlcMediaPlayer);
	//Free the media_player
	libvlc_media_player_release (_pVlcMediaPlayer);
	libvlc_release(_pVlcInstance);

	_bEnable = false;
	_pQueue = NULL;
}

bool DirectCameraConnector::isStillProducingImages()
{
	return _bEnable;
}

void* DirectCameraConnector::lock(void *data, void**p_pixels)
{
	struct ctx *ctx = (struct ctx*) data;
	if(_pQueue)
	{
		*p_pixels = ctx->puchPixels;
	}
	return NULL;
}

void DirectCameraConnector::unlock(void *data, void *id, void * const *p_pixels)
{
	assert(id == NULL);
}

void DirectCameraConnector::display(void *data, void *id)
{
	if(_pQueue)
	{
		struct ctx *ctx = (struct ctx*)data;
		IplImage *pImageDisplay = ctx->pImage;
		cv::Mat matImage(pImageDisplay, false);
		if(!matImage.empty())
		{
			opencctv::Image* pImage = new opencctv::Image();
			std::vector<unsigned char> vTemp;
			cv::imencode(".jpg", matImage, vTemp);
			//cv::imwrite("test.jpg",matImage);
			pImage->setImageData(vTemp);
			pImage->setTimestamp(currentDateTime());
			_pQueue->push(pImage);
		}
	}
}

// Get current date/time in ISO 8601 format (YYYYMMDDTHHmmssZ)
std::string DirectCameraConnector::currentDateTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y%m%dT%H%M%SZ", &tstruct);
    return buf;
}

DirectCameraConnector::~DirectCameraConnector() {
	if(_pContext)
	{
		delete _pContext;
		_pContext = NULL;
	}
}

