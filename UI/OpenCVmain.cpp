
#include "../LibISR/LibISR.h"

#include "ImageSourceEngine.h"
#include "OpenNIEngine.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"

#pragma comment( lib, "opencv_core2410.lib" )
#pragma comment( lib, "opencv_highgui2410.lib" )

#include "../LibISRUtils/IOUtil.h"
#include "../LibISRUtils/Timer.h"
#include "../LibISRUtils/NVTimer.h"

using namespace LibISR::Engine;
using namespace LibISR::Objects;
using namespace LibISRUtils;

void main(int argc, char** argv)
{
	//const char *colorImgSource = "../Data/K1_cut/c-%04i.ppm";
	//const char *depthImgSource = "../Data/K1_cut/d-%04i.pgm";
	//const char *calibFile = "../Data/Calib_kinect1.txt";

	//const char *colorImgSource = "E:/Data/k1_cut/c-%04i.ppm";
	//const char *depthImgSource = "E:/Data/k1_cut/d-%04i.pgm";
	//const char *calibFile = "../Data/Calib_kinect1.txt";

	const char *colorImgSource = "E:/Libisr/k1_cut/cr0-%04i.ppm";
	const char *depthImgSource = "E:/Libisr/k1_cut/d-%04i.pgm";
	const char *calibFile = "../Data/calib.txt";

	const char *sdfFile = "../Data/newCut.bin";

	const char* histogram_rgb = "../Data/color.ppm";
	const char* histogram_mask = "../Data/mask.ppm";

	ImageSourceEngine *imageSource = new ImageFileReader(calibFile, colorImgSource, depthImgSource);
	//ImageSourceEngine *imageSource = new OpenNIEngine(calibFile,NULL,true);

	ISRLibSettings isrSettings;
	isrSettings.noHistogramDim = 16;
	isrSettings.noTrackingObj = 1;
	isrSettings.singleAappearanceModel = true;
	isrSettings.useGPU = false;

	ISRCoreEngine *coreEngine = new ISRCoreEngine(&isrSettings, &imageSource->calib, imageSource->getDepthImageSize(), imageSource->getRGBImageSize());

	coreEngine->shapeUnion->loadShapeFromFile(sdfFile, Vector3i(DT_VOL_SIZE, DT_VOL_SIZE, DT_VOL_SIZE), 0);
	coreEngine->shapeUnion->shareSDFWithExistingShape(*coreEngine->shapeUnion->getShape(0), 1);

	// initialize color histogram from mask image
	ISRUChar4Image *histogramimage = new ISRUChar4Image(imageSource->getDepthImageSize(), false);
	ISRUChar4Image *histogrammask = new ISRUChar4Image(imageSource->getDepthImageSize(), false);
	if (!ReadImageFromFile(histogramimage, histogram_rgb)) { printf("wrong!\n"); return; }
	if (!ReadImageFromFile(histogrammask, histogram_mask)) { printf("wrong!\n"); return;}
	coreEngine->frame->histogram->buildHistogram(histogramimage, histogrammask);

	// initialized poses are [T' R']'
	float pose1[6] = { 0.5119f, -0.1408f, 0.7854f, 0.0f, -0.637070260807493f, 0.0f };
	float pose2[6] = { 0.6687f, 0.5081f, 0.1909f, 0.5469f, 0.9473f, -0.9473f };
	
	coreEngine->trackingState->setInvHFromParam(pose1, 0);
	coreEngine->trackingState->setInvHFromParam(pose2, 1);

	//////////////////////////////////////////////////////////////////////////
	// some tests
	//////////////////////////////////////////////////////////////////////////

	ISRLowlevelEngine* lengine = new ISRLowlevelEngine_CPU();
	ISRVisualisationEngine* vengine = new ISRVisualisationEngine_CPU();

	ISRUCharImage* maskimg = new ISRUCharImage(Vector2i(640, 480), false);
	ISRFloatImage* dtimg = new ISRFloatImage(Vector2i(640, 480), false);
	ISRFloat4Image* ptimg = new ISRFloat4Image(Vector2i(640, 480), false);
	ISRFloat2Image* minmaximg = new ISRFloat2Image(Vector2i(640, 480), false);

	dtimg->Clear(0);

	Vector4i bb = lengine->findBoundingBoxFromCurrentState(coreEngine->trackingState, coreEngine->getView()->calib->intrinsics_d.A, ptimg->noDims);

	vengine->updateMinmaxmImage(minmaximg, coreEngine->trackingState->getPose(0)->getH(), coreEngine->getView()->calib->intrinsics_d.A, ptimg->noDims);
	vengine->renderContour(maskimg, ptimg, minmaximg, coreEngine->trackingState->getPose(0)->getInvH(), coreEngine->shapeUnion->getShape(0), coreEngine->getView()->calib->intrinsics_d.getParam());
	lengine->computeSDFFromMask(dtimg, maskimg, bb);

	


	//////////////////////////////////////////////////////////////////////////
	// opencv interface stuff
	//////////////////////////////////////////////////////////////////////////

	cvNamedWindow("Depth", 0);
	IplImage* depthFrame = cvCreateImage(cvSize(640, 480), 8, 4);

	Vector3f cpt[4];	cpt[0] = Vector3f(0, 0, 0);	cpt[1] = Vector3f(0.1, 0, 0);	cpt[2] = Vector3f(0, 0.1, 0);	cpt[3] = Vector3f(0, 0, 0.1);
	CvPoint cvpt[4];Vector3f ipt[4];
	CvScalar color[3];	color[0] = CV_RGB(0, 0, 255);	color[1] = CV_RGB(0, 255, 0);	color[2] = CV_RGB(255, 0, 0);
	CvScalar bbcolor = CV_RGB(255,255,0);

	int key;
	int count = 0;

	Matrix3f A = coreEngine->getView()->calib->intrinsics_d.A;
	Matrix3f H = coreEngine->getView()->calib->homo_depth_to_color.H;
	Vector3f T = coreEngine->getView()->calib->homo_depth_to_color.T*0.001;

	StopWatchInterface *timer;
	sdkCreateTimer(&timer);
	float processedTime = 0;


	ISRLowlevelEngine* lowengine = new ISRLowlevelEngine_GPU();

	while ((key = cvWaitKey(10)) != 27)
	{

		if (!imageSource->hasMoreImages()) return;
		imageSource->getImages(coreEngine->getView());


		//sdkResetTimer(&timer); sdkStartTimer(&timer);
		coreEngine->processFrame();
		//sdkStopTimer(&timer); processedTime += sdkGetTimerValue(&timer);

		printf("\rAverage Tracking Time : [%f] ms = [%d] fps\tEnergy = %f", processedTime / count, (int)(count*1000 / processedTime),coreEngine->getEnergy());

		Vector4i bb = coreEngine->frame->imgHierarchy->levels[0].boundingbox;
		memcpy(depthFrame->imageData, (char*)coreEngine->getView()->alignedRgb->GetData(false), 640 * 480 * sizeof(char) * 4);
		//memcpy(depthFrame->imageData, (char*)coreEngine->getRenderingState()->outputImage->GetData(false), 640 * 480 * sizeof(char) * 4); H.setIdentity(); T = Vector3f(0, 0, 0);

		//// draw the axis on object
		//Matrix4f M = coreEngine->trackingState->getPose(0)->getH();
		//for (int i = 0; i < 4; i++)
		//{
		//	ipt[i] = H*(A*(M*cpt[i])) + T;
		//	cvpt[i].x = ipt[i].x / ipt[i].z;
		//	cvpt[i].y = ipt[i].y / ipt[i].z;
		//}
		//for (int i = 0; i < 3; i++) cvDrawLine(depthFrame, cvpt[0], cvpt[i + 1], color[i], 2);
		//
		//M = coreEngine->trackingState->getPose(1)->getH();
		//for (int i = 0; i < 4; i++)
		//{
		//	ipt[i] = H*(A*(M*cpt[i])) + T;
		//	cvpt[i].x = ipt[i].x / ipt[i].z;
		//	cvpt[i].y = ipt[i].y / ipt[i].z;
		//}
		//for (int i = 0; i < 3; i++) cvDrawLine(depthFrame, cvpt[0], cvpt[i + 1], color[i], 2);
		
		//// draw the bounding box
		//CvPoint p1, p2, p3, p4;
		//p1.x = bb.x; p1.y = bb.y; 
		//p2.x = bb.z; p2.y = bb.y;
		//p3.x = bb.z; p4.y = bb.w;
		//p4.x = bb.x, p3.y = bb.w;
		//
		//cvDrawLine(depthFrame, p1, p2, bbcolor, 2);
		//cvDrawLine(depthFrame, p2, p3, bbcolor, 2);
		//cvDrawLine(depthFrame, p3, p4, bbcolor, 2);
		//cvDrawLine(depthFrame, p4, p1, bbcolor, 2);

		cvShowImage("Depth", depthFrame);

		//char tmpchar[200];
		//sprintf(tmpchar, outName, count);
		//cvSaveImage(tmpchar, depthFrame);

		count++;
	}
	cvDestroyAllWindows();
}