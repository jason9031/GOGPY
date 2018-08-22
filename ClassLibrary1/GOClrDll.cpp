// ������ DLL �ļ���
#using <system.drawing.dll>
#include "stdafx.h"
#include "GOClrDll.h"
#include "opencv/cv.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

using namespace std;
using namespace cv;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;

using namespace GOClrDll;



System::Drawing::Bitmap^ MatToBitmap(const cv::Mat& img)
{
	if (img.type() != CV_8UC3)
	{
		throw gcnew NotSupportedException("Only images of type CV_8UC3 are supported for conversion to Bitmap");
	}

	//create the bitmap and get the pointer to the data
	PixelFormat fmt(PixelFormat::Format24bppRgb);
	Bitmap ^bmpimg = gcnew Bitmap(img.cols, img.rows, fmt);

	BitmapData ^data = bmpimg->LockBits(System::Drawing::Rectangle(0, 0, img.cols, img.rows), ImageLockMode::WriteOnly, fmt);

	Byte *dstData = reinterpret_cast<Byte*>(data->Scan0.ToPointer());

	unsigned char *srcData = img.data;

	for (int row = 0; row < data->Height; ++row)
	{
		memcpy(reinterpret_cast<void*>(&dstData[row*data->Stride]), reinterpret_cast<void*>(&srcData[row*img.step]), img.cols*img.channels());
	}

	bmpimg->UnlockBits(data);

	return bmpimg;
}

static vector<cv::Point> FindBiggestContour(cv::Mat src){
	int max_area_contour_idx = 0;
	double max_area = -1;
	vector<vector<cv::Point> >contours;
	findContours(src,contours,RETR_LIST,CHAIN_APPROX_SIMPLE);
	//handle case if no contours are detected
	if(0 == contours.size())
		return vector<cv::Point>(NULL);
	for (uint i=0;i<contours.size();i++){
		double temp_area = contourArea(contours[i]);
		if (max_area < temp_area ){
			max_area_contour_idx = i;
			max_area = temp_area;
		}
	}
	return contours[max_area_contour_idx];
}

Bitmap^  GOClrClass::testMethod(cli::array<unsigned char>^ pCBuf1)
{
	////////////////////////////////������cli::array<unsigned char>ת��Ϊcv::Mat/////////////////////////
	pin_ptr<System::Byte> p1 = &pCBuf1[0];
	unsigned char* pby1 = p1;
	cv::Mat img_data1(pCBuf1->Length,1,CV_8U,pby1);
	cv::Mat img_object = cv::imdecode(img_data1,IMREAD_UNCHANGED);
	if (!img_object.data)
		return nullptr;

	////////////////////////////////////////////OpenCV���㷨�������////////////////////////////////////
	vector<vector<cv::Point> > contours;
	vector<Vec4i> hierarchy;
	Mat drawing = img_object.clone();
	cvtColor(img_object,img_object,COLOR_BGR2GRAY);
	cv::threshold(img_object,img_object,100,255,THRESH_OTSU);
	////Ѱ���������
	vector<cv::Point> biggestContour = FindBiggestContour(img_object);
	vector<cv::Point> approxContour;
	if (0==biggestContour.size())
	    return nullptr;

	//////�����������������б�ı߽��(���ú�ɫ��
	RotatedRect minRect = minAreaRect(biggestContour);
	Point2f rect_points[4];
	minRect.points( rect_points );
	for( int j = 0; j < 4; j++ )
		line( drawing, rect_points[j], rect_points[(j+1)%4], Scalar(0,0,255), 1, 8 );

	////�������������������4����������ߣ�������ɫ��
    double peri  = arcLength(biggestContour,true);
	approxPolyDP(biggestContour,approxContour,0.02*peri,true);
	if (approxContour.size() == 4)//�ĸ���
	{
		for( int j = 0; j < 4; j++ )
			line( drawing, approxContour[j], approxContour[(j+1)%4], Scalar(0,255,0), 1, 8 );
	}

	/////////////////////////��cv::Matת��ΪBitmap(ֻ�ܴ���cv_8u3��ʽ���ݣ�///////////////////////////////
	if (!drawing.data)
		return nullptr;
	Bitmap^ bitmap = MatToBitmap(drawing);
	return bitmap;
}


Bitmap^  GOClrClass::fetchresult(cli::array<unsigned char>^ pCBuf1)
{
	////////////////////////////////������cli::array<unsigned char>ת��Ϊcv::Mat/////////////////////////
	pin_ptr<System::Byte> p1 = &pCBuf1[0];
	unsigned char* pby1 = p1;
	cv::Mat img_data1(pCBuf1->Length,1,CV_8U,pby1);
	cv::Mat img_object = cv::imdecode(img_data1,IMREAD_UNCHANGED);
	if (!img_object.data)
		return nullptr;

	////////////////////////////////////////////OpenCV���㷨�������////////////////////////////////////
	vector<vector<cv::Point> > contours;
	vector<Vec4i> hierarchy;
	Mat drawing = img_object.clone();
	cvtColor(img_object,img_object,COLOR_BGR2GRAY);
	cv::threshold(img_object,img_object,100,255,THRESH_OTSU);
	////Ѱ���������
	vector<cv::Point> biggestContour = FindBiggestContour(img_object);
	vector<cv::Point> approxContour;
	if (0==biggestContour.size())
		return nullptr;

	////�������������������4����������ߣ�������ɫ��
	double peri  = arcLength(biggestContour,true);
	approxPolyDP(biggestContour,approxContour,0.02*peri,true);

	////͸�ӱ仯
	Point2f src_vertices[4];
	Point2f dst_vertices[4];
	if (approxContour.size() == 4)//�ĸ���
	{  
		float radiusCircle;
		Point2f centerCircle;
		minEnclosingCircle(approxContour,centerCircle,radiusCircle);
		for( int j = 0; j < 4; j++ )
		{
			if (approxContour[j].x < centerCircle.x && approxContour[j].y < centerCircle.y)
				src_vertices[0] = approxContour[j];
			if (approxContour[j].x > centerCircle.x && approxContour[j].y < centerCircle.y)
				src_vertices[1] = approxContour[j];
			if (approxContour[j].x < centerCircle.x && approxContour[j].y > centerCircle.y)
				src_vertices[2] = approxContour[j];
			if (approxContour[j].x > centerCircle.x && approxContour[j].y > centerCircle.y)
				src_vertices[3] = approxContour[j];
		}
		float dstWidth = std::min((src_vertices[1].x - src_vertices[0].x),(src_vertices[3].x - src_vertices[2].x));
		float dstHeight= std::min((src_vertices[3].y - src_vertices[1].y),(src_vertices[2].y - src_vertices[0].y));
		dst_vertices[0] = Point2f(0, 0);
		dst_vertices[1] = Point2f(dstWidth,0);
		dst_vertices[2] = Point2f(0,dstHeight);
		dst_vertices[3] = Point2f(dstWidth,dstHeight);
		Mat warpMatrix = getPerspectiveTransform(src_vertices, dst_vertices);
		warpPerspective(drawing, drawing, warpMatrix, drawing.size(), INTER_LINEAR, BORDER_CONSTANT);
		drawing = drawing(Rect(0,0,dstWidth,dstHeight));
	}
	else
	{
		return nullptr;
	}
	/////////////////////////��cv::Matת��ΪBitmap(ֻ�ܴ���cv_8u3��ʽ���ݣ�///////////////////////////////
	if (!drawing.data)
		return nullptr;
	Bitmap^ bitmap = MatToBitmap(drawing);
	return bitmap;
}


