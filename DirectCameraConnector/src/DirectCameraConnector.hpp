/*
 * DirectCameraConnector.hpp
 *
 *  Created on: Oct 6, 2015
 *      Author: anjana
 */

#ifndef DIRECTCAMERACONNECTOR_HPP_
#define DIRECTCAMERACONNECTOR_HPP_

#include "opencctv/api/VmsConnector.hpp"

#include <vlc/vlc.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>

#include <cassert>

#include <vector>
#include <sstream>

struct ctx
{
	IplImage *pImage;
	uchar *puchPixels;
};

class DirectCameraConnector: public opencctv::api::VmsConnector
{
private:
	ctx* _pContext;
	std::vector<unsigned char> _vCurrentImageData;
	libvlc_instance_t* _pVlcInstance;
	libvlc_media_t* _pVlcMedia;
	libvlc_media_player_t* _pVlcMediaPlayer;
	static void* lock(void *data, void**p_pixels);
	static void unlock(void *data, void *id, void * const *p_pixels);
	static void display(void *data, void *id);
	static std::string currentDateTime();
public:
	DirectCameraConnector();
	bool init(const opencctv::api::VmsConnectInfo& info, const std::string sPathToAnalyticDir);
	void produceImageObjects(opencctv::ConcurrentQueue<opencctv::Image>* pQueue);
	bool isStillProducingImages();
	virtual ~DirectCameraConnector();
};

extern "C" DirectCameraConnector* create() {
	return new DirectCameraConnector;
}

extern "C" void destroy(DirectCameraConnector* p) {
	if (p) {
		delete p;
		p = NULL;
	}
}

#endif /* DIRECTCAMERACONNECTOR_HPP_ */
