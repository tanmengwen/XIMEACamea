#include "stdafx.h"

#ifdef WIN32
#include "xiApi.h"
#else
#include <m3api/xiApi.h>
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <list>

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;


list<XI_IMG> imagesVectorR;
list<XI_IMG> imagesVectorL;
#define EXPECTED_IMAGES 1000
#define HandleResult(res,place) if (res!=XI_OK) {printf("Error after %s (%d)\n",place,res);goto finish;}
HANDLE xiHLeft = NULL;
HANDLE xiHRight = NULL;

bool bSaved = true;
int totalFrameNum = -1;
VideoWriter _videoWriterLeft;
VideoWriter _videoWriterRight;



void saveLeftImages()
{
	/*namedWindow("video",1);*/
	int num =0;
	while(bSaved){

		if(imagesVectorL.size()>0)
		{
			XI_IMG m = imagesVectorL.front();

			cv::Mat img = cv::Mat(1024, 1280, CV_8UC3, m.bp);

			_videoWriterLeft << img;

			imagesVectorL.pop_front();

			free(m.bp);

			num++;

		}

		if (num == EXPECTED_IMAGES)
		{
			break;
		}
	}
	_videoWriterLeft.release();
	waitKey(0);
}

void saveRightImages()
{
	/*namedWindow("video",1);*/
	int num =0;
	while(bSaved){

		if(imagesVectorR.size()>0)
		{
			 XI_IMG m = imagesVectorR.front();

			cv::Mat img = cv::Mat(1024, 1280, CV_8UC3, m.bp);

			_videoWriterRight << img;

			imagesVectorR.pop_front();

			free(m.bp);

			num++;

		}

		if (num == EXPECTED_IMAGES)
		{
			break;
		}
	}
	_videoWriterRight.release();
	waitKey(0);
}
int CaptureImage( IplImage** image, PDWORD fn, XI_IMG img )
{
	int mvret = XI_OK;

	(*image) = cvCreateImage(cvSize( img.width, img.height), IPL_DEPTH_8U, 3); 
	memcpy( (*image)->imageData, img.bp, img.bp_size);
	*fn = img.nframe;

	return XI_OK;
}
void leftCamera(){
	XI_IMG image;
	image.size = sizeof(XI_IMG);
	image.bp = NULL;
	image.bp_size = 0;
	XI_RETURN stat = XI_OK;
	// Get number of camera devices
	DWORD dwNumberOfDevices = 0;
	stat = xiGetNumberDevices(&dwNumberOfDevices);
	//HandleResult(stat,"xiGetNumberDevices (no camera found)");


	// Retrieving a handle to the camera device 
	stat = xiOpenDevice(0, &xiHLeft);

	// Setting "exposure" parameter (10ms=10000us)
	stat = xiSetParamInt(xiHLeft, XI_PRM_EXPOSURE, 3600);

	stat = xiSetParamInt(xiHLeft, XI_PRM_IMAGE_DATA_FORMAT, XI_RGB24);

	stat = xiSetParamInt(xiHLeft, XI_PRM_GPI_SELECTOR, 1);
	stat = xiSetParamInt(xiHLeft, XI_PRM_GPI_MODE, XI_GPI_TRIGGER);
	stat = xiSetParamInt(xiHLeft, XI_PRM_TRG_SOURCE, XI_TRG_EDGE_RISING);
	stat = xiSetParamInt(xiHLeft, XI_PRM_TRG_SELECTOR, XI_TRG_SEL_FRAME_START);

	//stat = xiSetParamInt(xiHLeft, XI_PRM_AEAG, 1);
	//stat = xiSetParamInt(xiHLeft, XI_PRM_AUTO_WB, 1);

	stat = xiSetParamInt(xiHLeft, XI_PRM_BUFFERS_QUEUE_SIZE, 0);

	stat = xiSetParamInt(xiHLeft, XI_PRM_BUFFER_POLICY, XI_BP_SAFE);

	stat = xiStartAcquisition(xiHLeft);
	float frameRate=0;
	double t = (double)cvGetTickCount();
	for (int images=0;images < EXPECTED_IMAGES;images++)
	{
		// getting image from camera
		stat = xiGetImage(xiHLeft, 50000, &image);
		
		xiGetParamFloat(xiHLeft,XI_PRM_FRAMERATE,&frameRate);
		printf("left ImageNum: %d -- nframe: %d frameRate:%f\n", images, image.nframe, frameRate);

		//printf("left ImageNum: %d -- nframe: %d \n", images, image.nframe);
		

		XI_IMG newImage = image;
		int size = (image.width + image.padding_x / 3) * image.height * 3;
		newImage.bp = malloc(size);
		memcpy(newImage.bp, image.bp, size);
		imagesVectorL.push_back(newImage);

		//cv::Mat img = cv::Mat(1024, 1280, CV_8UC3, image.bp);
		//_videoWriterLeft << img;
	}
	t = (double)cvGetTickCount() - t;
	printf( "run time = %gms\n", t/(cvGetTickFrequency()*1000) );
	//_videoWriterLeft.release();
	xiCloseDevice(xiHLeft);
}
void rightCamera(){
	
	XI_RETURN stat = XI_OK;
	// Get number of camera devices
	DWORD dwNumberOfDevices = 1;
	stat = xiGetNumberDevices(&dwNumberOfDevices);
	//HandleResult(stat,"xiGetNumberDevices (no camera found)");


	// Retrieving a handle to the camera device 
	stat = xiOpenDevice(1, &xiHRight);

	// Setting "exposure" parameter (10ms=10000us)
	stat = xiSetParamInt(xiHRight, XI_PRM_EXPOSURE, 10*1000);

	stat = xiSetParamInt(xiHRight, XI_PRM_IMAGE_DATA_FORMAT, XI_RGB24);

	stat = xiSetParamInt(xiHRight, XI_PRM_GPI_SELECTOR, 1);
	stat = xiSetParamInt(xiHRight, XI_PRM_GPI_MODE, XI_GPI_TRIGGER);
	stat = xiSetParamInt(xiHRight, XI_PRM_TRG_SOURCE, XI_TRG_EDGE_RISING);
	stat = xiSetParamInt(xiHRight, XI_PRM_TRG_SELECTOR, XI_TRG_SEL_FRAME_START);

	/*stat = xiSetParamInt(xiHRight, XI_PRM_AEAG, 1);*/
	/*stat = xiSetParamInt(xiHRight, XI_PRM_AUTO_WB, 1);*/

	stat = xiSetParamInt(xiHRight, XI_PRM_BUFFERS_QUEUE_SIZE, 0);

	stat = xiSetParamInt(xiHRight, XI_PRM_BUFFER_POLICY, XI_BP_SAFE);

	stat = xiStartAcquisition(xiHRight);
	float frameRate=0;
	double t = (double)cvGetTickCount();

	XI_IMG image;
	image.size = sizeof(XI_IMG);
	image.bp = NULL;
	image.bp_size = 0;

	for (int images=0;images < EXPECTED_IMAGES;images++)
	{
		// getting image from camera
		stat = xiGetImage(xiHRight, 50000, &image);

		//printf("right ImageNum: %d -- nframe: %d \n", images, image.nframe);

		xiGetParamFloat(xiHRight,XI_PRM_FRAMERATE,&frameRate);
		printf("right ImageNum: %d -- nframe: %d frameRate:%f\n", images, image.nframe, frameRate);


		XI_IMG newImage = image;
		int size = (image.width + image.padding_x / 3) * image.height * 3;
		newImage.bp = malloc(size);
		memcpy(newImage.bp, image.bp, size);
		imagesVectorR.push_back(newImage);

		//cv::Mat img = cv::Mat(1024, 1280, CV_8UC3, image.bp);
		//_videoWriterRight << img;
	}
	//_videoWriterRight.release();
	xiCloseDevice(xiHRight);
}

int _tmain(int argc,char ** argv)
{

	HANDLE hThreadleft;//线程句柄
	DWORD threadIDleft;//线程IdS
	HANDLE hThreadright;//线程句柄
	DWORD threadIDright;//线程Id
	
	HANDLE hSaveRThread;//线程句柄
	DWORD threadIDR;//线程Id

	HANDLE hSaveLThread;//线程句柄
	DWORD threadIDL;//线程Id
	
	
	char flag = *argv[1];
	if (flag== 'r')
	{
		cout<<"Right Camera"<<endl;
		_videoWriterRight = VideoWriter("Videoright.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30.0, cv::Size(1280, 1024));
		hThreadright = CreateThread(
			NULL,									 // SD
			0,									 // initial stack size
			(LPTHREAD_START_ROUTINE)rightCamera,    // thread function
			NULL,									 // thread argument
			0,									 // creation option
			&threadIDright								 // thread identifier
			);

		hSaveRThread = CreateThread(
			NULL,									 // SD
			0,									 // initial stack size
			(LPTHREAD_START_ROUTINE)saveRightImages,    // thread function
			NULL,									 // thread argument
			0,									 // creation option
			&threadIDR							 // thread identifier
			);
	} 
	if (flag== 'l')
	{
		cout<<"Left Camera"<<endl;
		_videoWriterLeft = VideoWriter("Videoleft.avi", CV_FOURCC('M', 'J', 'P', 'G'), 25.0, cv::Size(1280, 1024));
		hThreadleft = CreateThread(
			NULL,									 // SD
			0,									 // initial stack size
			(LPTHREAD_START_ROUTINE)leftCamera,    // thread function
			NULL,									 // thread argument
			0,									 // creation option
			&threadIDleft								 // thread identifier
			);
		hSaveLThread = CreateThread(
			NULL,									 // SD
			0,									 // initial stack size
			(LPTHREAD_START_ROUTINE)saveLeftImages,    // thread function
			NULL,									 // thread argument
			0,									 // creation option
			&threadIDL							 // thread identifier
			);
	} 
	 

	char str[8];
	cin.getline(str, 5);

	return 0;
}