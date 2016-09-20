// http://stackoverflow.com/questions/13876366/how-to-take-kinect-video-image-and-depth-image-with-opencv-c

//Includes
#include <GL/freeglut.h>
#include <MSHTML.h>
#include <NuiApi.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "TextureManager.h"
#include "GLUtilities.h"
#include <opencv/cv.h>
//#include <opencv/highgui.h>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/highgui/highgui_c.h"
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include "NuiImageBuffer.h"

#define COLOR_WIDTH 640    
#define COLOR_HIGHT 480    
#define DEPTH_WIDTH 640    
#define DEPTH_HIGHT 480     
#define SKELETON_WIDTH 640    
#define SKELETON_HIGHT 480    
#define CHANNEL 3

#define w 500

using namespace cv;
using namespace std;

//Global Variables
INuiSensor* context = NULL;
HANDLE h1;
HANDLE h2;
HANDLE h3;
HANDLE h4;

IplImage* color = NULL;
IplImage* depth = NULL;
IplImage* filter = NULL;
IplImage* final = NULL;
IplImage* imgBackground = NULL;

CvMemStorage *storage;
int levels = 3;
CvSeq* contours = 0;


CvCapture* g_Capture;
IplImage *image;
clock_t start;
clock_t diff;
int msec; 

BYTE buf[DEPTH_WIDTH * DEPTH_HIGHT * CHANNEL];

int drawColor(HANDLE h, IplImage* color)    
{

	const NUI_IMAGE_FRAME * pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame(h, 0, &pImageFrame);
	if (FAILED(hr)) 
	{
		cout << "Get Image Frame Failed" << endl;
		return -1;
	}

	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);
	if (LockedRect.Pitch != NULL)
	{
		BYTE * pBuffer = (BYTE*) LockedRect.pBits;
		cvSetData(color, pBuffer, LockedRect.Pitch);
	}
	cvShowImage("color image", color);
	NuiImageStreamReleaseFrame(h, pImageFrame);
	return 0;
}


int drawDepth(HANDLE h, IplImage* depth)
{
	const NUI_IMAGE_FRAME * pImageFrame = NULL;
	HRESULT hr = NuiImageStreamGetNextFrame(h, 0, &pImageFrame);
	if (FAILED(hr))
	{
		cout << "Get Image Frame Failed" << endl;
		return -1;
	}

	INuiFrameTexture * pTexture = pImageFrame->pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect(0, &LockedRect, NULL, 0);
	if (LockedRect.Pitch != 0)
	{

		USHORT * pBuff = (USHORT*) LockedRect.pBits;
		for (int i = 0; i < DEPTH_WIDTH * DEPTH_HIGHT; i++)
		{
			BYTE index = pBuff[i] & 0x07;
			USHORT realDepth = (pBuff[i] & 0xFFF8) >> 3;
			BYTE scale = 255 - (BYTE)(256 * realDepth / 0x0fff);
			buf[CHANNEL * i] = buf[CHANNEL * i + 1] = buf[CHANNEL * i + 2] = 0;

			// Color by Player Index
			switch (index)
			{
			case 0:	//No Player Index -> set to white
				buf[CHANNEL * i] = 255;
				buf[CHANNEL * i + 1] = 255;
				buf[CHANNEL * i + 2] = 255;
				break;
				//Players 1-7 set to black
			case 1:
				buf[CHANNEL * i] = 0;
				buf[CHANNEL * i + 1] = 0;
				buf[CHANNEL * i + 2] = 0;
				break;
			case 2:
				buf[CHANNEL * i] = 0;
				buf[CHANNEL * i + 1] = 0;
				buf[CHANNEL * i + 2] = 0;
				break;
			case 3:
				buf[CHANNEL * i] = 0;
				buf[CHANNEL * i + 1] = 0;
				buf[CHANNEL * i + 2] = 0;
				break;
			case 4:
				buf[CHANNEL * i] = 0;
				buf[CHANNEL * i + 1] = 0;
				buf[CHANNEL * i + 2] = 0;
				break;
			case 5:
				buf[CHANNEL * i] = 0;
				buf[CHANNEL * i + 1] = 0;
				buf[CHANNEL * i + 2] = 0;
				break;
			case 6:
				buf[CHANNEL * i] = 0;
				buf[CHANNEL * i + 1] = 0;
				buf[CHANNEL * i + 2] = 0;
				break;
			case 7:
				buf[CHANNEL * i] = 0;
				buf[CHANNEL * i + 1] = 0;
				buf[CHANNEL * i + 2] = 0;
				break;
			}
		}
		cvSetData(depth, buf, DEPTH_WIDTH * CHANNEL);
	}

	diff = clock() - start;

	msec = diff * 1000 / CLOCKS_PER_SEC;

	// Movie loop
	if(msec/1000 <-1){

		// Capture next frame
		if(cvQueryFrame(g_Capture)){

			image = cvQueryFrame(g_Capture);

			// Convert to RGB
			//cvCvtColor(image, image, CV_BGR2RGB);

			cvShowImage("Pilobolus Shadows", image);

			DWORD ms = 50;
			Sleep(ms);
		}
	}
	// Shadows loop
	if(msec/1000 >=0 ){			


		// Filter to remove pixelation
		cvMorphologyEx(depth, depth, 0, 0, CV_MOP_OPEN,2);
		cvSmooth(depth, depth, CV_MEDIAN, 11);		
		//cvCopy(depth,filter);
		//cvDilate(filter, depth, 0, 1);
		cvMorphologyEx(depth, depth, 0, 0, CV_MOP_CLOSE,2);

		//Find contours
		cvCvtColor(depth, filter, CV_RGB2GRAY);
		cvFindContours( filter, storage, &contours, sizeof(CvContour),CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
		contours = cvApproxPoly( contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 1.5, 1);
		cvDrawContours(final, contours, CV_RGB(255,0,0), CV_RGB(255,0,0), 10, CV_FILLED, CV_AA, cvPoint(0,0));
		//CV_RGB(255,165,0) CV_FILLED CV_AA cvDrawContours(final, contours, CV_RGB(0,0,0), CV_RGB(0,0,0), 10, 1, 8, cvPoint(0,0));
		//cvShowImage("Pilobolus Shadows", final);
		
		CvScalar currentBackgroundPixel;
		for (int i=0; i<color->width; i++) 
		{
			for (int j=0; j<color->height; j++) 
			{
				CvScalar currentPixel = cvGet2D(final, j, i);
				// compare the RGB values of the pixel. if white, set to black
				if (currentPixel.val[1] > 250 ) //&& currentPixel.val[1] > 250 && currentPixel.val[2] > 250
				{
					currentBackgroundPixel.val[0] = 0;
					currentBackgroundPixel.val[1] = 0;
					currentBackgroundPixel.val[2] = 0;
					cvSet2D(color, j, i, currentBackgroundPixel);
				}
				else{ //if not white, use background image
					
					// copy the corresponding pixel from background
					currentBackgroundPixel = cvGet2D(imgBackground, j, i);
					cvSet2D(color, j, i, currentBackgroundPixel);

				}
			}
		}
		
		//cvCopy(filter,final);
		cvShowImage("Pilobolus Shadows", color);	
		//cvShowImage("color image", color);
	}

	// Resart timer
	if(msec/1000 > 600){			
		cvReleaseCapture(&g_Capture);
		start = clock();
		g_Capture = cvCaptureFromFile("video1.avi");
	}

	cvSet(final, CV_RGB(255,255,255));	//white
	NuiImageStreamReleaseFrame(h, pImageFrame);
	return 0;
}

bool initializeKinect()
{
	//Check if there are any Kinect sensors connected and obtain the number
	int numKinects = 0;
	HRESULT hr = NuiGetSensorCount( &numKinects );
	if ( FAILED(hr) || numKinects<=0 )
	{
		std::cout << "No Kinect device found." << std::endl;
		return false;
	}
	else{
		std::cout << "Found " << numKinects << " Kinect device(s)." << std::endl;
	}

	//Create the sensor object and set it to context. Only use the first device found.
	hr = NuiCreateSensorByIndex( 0, &context );
	if ( FAILED(hr) )
	{
		std::cout << "Failed to connect to Kinect device." << std::endl;
		return false;
	}

	//Initialize the sensor with color/depth/skeleton enabled
	DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
	hr = context->NuiInitialize( nuiFlags );
	if ( FAILED(hr) )
	{
		std::cout << "Failed to intialize Kinect: " << std::hex << (long)hr << std::dec << std::endl;
		return false;
	}

	//Open color and depth video streams for capturing. Resolution set to 640x480	
	h1 = CreateEvent(NULL, TRUE, FALSE, NULL);
	h2 = NULL;
	hr = context->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, h1, &h2);
	if ( FAILED(hr) )
	{
		std::cout << "Unable to create color stream: " << std::hex << (long)hr << std::dec << std::endl;
		return false;
	}	

	h3 = CreateEvent(NULL, TRUE, FALSE, NULL);
	h4 = NULL;
	hr = context->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, NUI_IMAGE_RESOLUTION_640x480, 0, 2, h3, &h4);
	if ( FAILED(hr) )
	{
		std::cout << "Unable to create depth stream: " << std::hex << (long)hr << std::dec << std::endl;
		return false;
	}

	//Enable skeleton tracking
	hr = context->NuiSkeletonTrackingEnable( NULL, 0 );
	if ( FAILED(hr) )
	{
		std::cout << "Unable to start tracking skeleton." << std::endl;
		return false;
	}

	std::cout << "Kinect Initialized Successfully" << std::endl;

	return true;
}

int main(int argc, char * argv[])
{
	color = cvCreateImage(cvSize(COLOR_WIDTH, COLOR_HIGHT), IPL_DEPTH_8U, 4);

	depth = cvCreateImage(cvSize(DEPTH_WIDTH, DEPTH_HIGHT),IPL_DEPTH_8U, CHANNEL);

	filter = cvCreateImage(cvSize(DEPTH_WIDTH, DEPTH_HIGHT),IPL_DEPTH_8U, 1);

	final = cvCreateImage(cvSize(DEPTH_WIDTH, DEPTH_HIGHT),IPL_DEPTH_8U, CHANNEL);

	imgBackground = cvCreateImage(cvSize(COLOR_WIDTH, COLOR_HIGHT), IPL_DEPTH_8U, 4);

	cvSet(final, CV_RGB(255,255,255));

	storage = cvCreateMemStorage(0);

	contours = 0;

	g_Capture = cvCaptureFromFile("video1.avi");

	imgBackground = cvLoadImage("background.png");

	//imgBackground = cvLoadImage("C:\\Users\\test\\Documents\\Visual Studio 2012\\Projects\\Win32Project1\\Release\\background.png");

	if (!imgBackground){
		printf("Image can NOT Load!!!\n");
		return 1;
	}

	start = clock();

	cvNamedWindow("color image", CV_WINDOW_AUTOSIZE);

	cvNamedWindow("Pilobolus Shadows", CV_WINDOW_NORMAL);

	cvSetWindowProperty("Pilobolus Shadows", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

	if (!initializeKinect()){
		return 1;
	}

	while (1)
	{
		//WaitForSingleObject(h1, INFINITE);
		//drawColor(h2, color);
		WaitForSingleObject(h3, INFINITE);
		drawDepth(h4, depth);

		//exit
		int c = cvWaitKey(1);
		if (c == 27 || c == 'q' || c == 'Q')
			break;
	}

	cvReleaseImageHeader(&depth);
	cvReleaseImageHeader(&color);
	cvReleaseCapture(&g_Capture);
	cvDestroyWindow("depth image");
	cvDestroyWindow("color image");

	context->NuiShutdown();

	return 0;

}