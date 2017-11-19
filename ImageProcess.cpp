#include "stdafx.h"
#include "ImageProcess.h"
#include <vector>
#include <algorithm>


static bool GetValue(int p[], int size, int &value)
{
	//数组中间的值
	int zxy = p[(size - 1) / 2];
	//用于记录原数组的下标
	int *a = new int[size];
	int index = 0;
	for (int i = 0; i < size; ++i)
		a[index++] = i;

	for (int i = 0; i < size - 1; i++)
		for (int j = i + 1; j < size; j++)
			if (p[i] > p[j]) {
				int tempA = a[i];
				a[i] = a[j];
				a[j] = tempA;
				int temp = p[i];
				p[i] = p[j];
				p[j] = temp;

			}
	int zmax = p[size - 1];
	int zmin = p[0];
	int zmed = p[(size - 1) / 2];

	if (zmax > zmed&&zmin < zmed) {
		if (zxy > zmin&&zxy < zmax)
			value = (size - 1) / 2;
		else
			value = a[(size - 1) / 2];
		delete[]a;
		return true;
	}
	else {
		delete[]a;
		return false;
	}

}

//使得坐标值不超过图像的最大值
int normaliseXY(int x, int y, int max_x, int max_y) {
	if (x >= max_x || x <= 0 || y >= max_y || y <= 0) //超出边界的部分用黑色填充
		return 0;
	return x;
}

//#include "opencv2/core/core.hpp"  
//#include"opencv2/highgui/highgui.hpp"  
//#include"opencv2/imgproc/imgproc.hpp"  
//void MatToCImage(cv::Mat &mat, CImage &cImage)
//{
//	//create new CImage  
//	int width = mat.cols;
//	int height = mat.rows;
//	int channels = mat.channels();
//
//	cImage.Destroy(); //clear  
//	cImage.Create(width, height, 8 * channels); //默认图像像素单通道占用1个字节  
//
//												//copy values  
//	uchar* ps;
//	uchar* pimg = (uchar*)cImage.GetBits(); //A pointer to the bitmap buffer  
//	int step = cImage.GetPitch();
//
//	for (int i = 0; i < height; ++i)
//	{
//		ps = (mat.ptr<uchar>(i));
//		for (int j = 0; j < width; ++j)
//		{
//			if (channels == 1) //gray  
//			{
//				*(pimg + i*step + j) = ps[j];
//			}
//			else if (channels == 3) //color  
//			{
//				for (int k = 0; k < 3; ++k)
//				{
//					*(pimg + i*step + j * 3 + k) = ps[j * 3 + k];
//				}
//			}
//		}
//	}
//}
//定义域核
double d(int i_k, int j_l , double sigama_d) {
	return exp(-1 * 
		(pow((i_k), 2) + pow((j_l), 2)) / (2 * pow(sigama_d, 2)));
}
//值域核
double r(int fij,int fkl, double sigama_r) {
	return exp(-1 *
		(pow(fij-fkl, 2) / (2 * pow(sigama_r, 2))));
}
//double w(int i, int j, int k, int l) {
//	return d(i,j,k,l,5)*r()
//}

int checkColorSpace(double x) {
	if (x > 255)
		return 255;
	if (x < 0)
		return 0;
	return x;
}

UINT ImageProcess::bilateralFilter(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	int maxSpan = param->maxSpan;
	int maxLength = (maxSpan * 2 + 1) * (maxSpan * 2 + 1);

	byte* pRealData = (byte*)param->src->GetBits();
	byte* pOriginData = (byte*)param->origin_src->GetBits();//param->origin_src指的是原图像
	int pit = param->src->GetPitch();
	int bitCount = param->src->GetBPP() / 8;

	int filter_length = param->bilateralFilterParam->filter_length;
	double sigma_d = param->bilateralFilterParam->sigma_d;
	double sigma_r = param->bilateralFilterParam->sigma_r;

	for (int a = startIndex; a <= endIndex; ++a) {
		int x = a % maxWidth;
		int y = a / maxWidth;
		for (int off = 0; off < bitCount; off++) {//bitCount = 1 意味着这是灰度图255位，若bitCount = 3 则意味着这是RGB（比如）图，将分别对3byte的数据一byte一byte赋值
			//获得输出像素点的值：g(i,j)
			double temp1 = 0, temp2 = 0;
			if (x >= filter_length && x < maxWidth - filter_length && y >= filter_length && y < maxHeight - filter_length) {//防止越界
				for (int k = -1* filter_length; k <= filter_length;k++)
					for (int l = -1* filter_length; l <= filter_length; l++) {
						//这里用原图像的像素，这样避免了多线程滤波的时候像素相互影响
						int fij = *(pRealData + pit * (y)+(x)* bitCount + off);
						int fkl = *(pRealData + pit * (y + l) + (x + k) * bitCount + off);
						temp1 += fkl*d(k, l, sigma_d)*r(fij, fkl, sigma_r);
						temp2 += d(k, l, sigma_d)*r(fij, fkl, sigma_r);
					}
			}
			*(pRealData + pit * y + x * bitCount + off) = checkColorSpace(temp1/temp2);
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_MEDIAN_FILTER, 1, NULL);
	return 0;
}

UINT ImageProcess::medianFilter(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	int maxSpan = param->maxSpan;
	int maxLength = (maxSpan * 2 + 1) * (maxSpan * 2 + 1);

	byte* pRealData = (byte*)param->src->GetBits();
	int pit = param->src->GetPitch();
	int bitCount = param->src->GetBPP() / 8;

	int *pixel = new int[maxLength];//存储每个像素点的灰度
	int *pixelR = new int[maxLength];
	int *pixelB = new int[maxLength];
	int *pixelG = new int[maxLength];
	int index = 0;
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int Sxy = 1;
		int med = 0;
		int state = 0;
		int x = i % maxWidth;
		int y = i / maxWidth;
		while (Sxy <= maxSpan)
		{
			index = 0;
			for (int tmpY = y - Sxy; tmpY <= y + Sxy && tmpY < maxHeight; tmpY++)
			{
				if (tmpY < 0) continue;
				for (int tmpX = x - Sxy; tmpX <= x + Sxy && tmpX < maxWidth; tmpX++)
				{
					if (tmpX < 0) continue;
					if (bitCount == 1)
					{
						pixel[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixelR[index++] = pixel[index];

					}
					else
					{
						pixelR[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 2);
						pixelG[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount + 1);
						pixelB[index] = *(pRealData + pit*(tmpY)+(tmpX)*bitCount);
						pixel[index++] = int(pixelB[index] * 0.299 + 0.587*pixelG[index] + pixelR[index] * 0.144);

					}
				}

			}
			if (index <= 0)
				break;
			if ((state = GetValue(pixel, index, med)) == 1)
				break;

			Sxy++;
		};

		if (state)
		{
			if (bitCount == 1)
			{
				*(pRealData + pit*y + x*bitCount) = pixelR[med];

			}
			else
			{
				*(pRealData + pit*y + x*bitCount + 2) = pixelR[med];
				*(pRealData + pit*y + x*bitCount + 1) = pixelG[med];
				*(pRealData + pit*y + x*bitCount) = pixelB[med];

			}
		}

	}



	delete[]pixel;
	delete[]pixelR;
	delete[]pixelG;
	delete[]pixelB;

	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_MEDIAN_FILTER, 1, NULL);
	return 0;
}




//在多线程程序中不能用全局变量，会有时序问题
//***
float BiCubicPoly(float x)
{
	float abs_x = abs(x);
	float a = -0.5;
	if (abs_x <= 1.0)
	{
		return (a + 2)*pow(abs_x, 3) - (a + 3)*pow(abs_x, 2) + 1;
	}
	else if (abs_x < 2.0)
	{
		return a*pow(abs_x, 3) - 5 * a*pow(abs_x, 2) + 8 * a*abs_x - 4 * a;
	}
	else
		return 0.0;
}


UINT ImageProcess::whiteBalance(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	//图像的整个宽度和高度
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	//GetBits() 获得数据区的指针
	byte* pRealData = (byte*)param->src->GetBits();
	//GetBPP() 返回每个像素所占的bit数，在这里/8意味着bitCount是每个像素所占的byte数
	int bitCount = param->src->GetBPP() / 8;
	//获得图像数据每一行的字节数
	//The pitch of the image.If the return value is negative, the bitmap is a bottom - up DIB and its origin is the lower left corner.
	//	If the return value is positive, the bitmap is a top - down DIB and its origin is the upper left corner.
	int pit = param->src->GetPitch();
	double R = param->R_sum;//整个图像的RGB分量总和
	double G = param->G_sum;//整个图像的RGB分量总和
	double B = param->B_sum;//整个图像的RGB分量总和
	//需要调整的RGB分量的增益  
	double KB = (R+ G+ B) / (3 * B);
	double KG = (R+ G+ B) / (3 * G);
	double KR = (R+ G+ B) / (3 * R);
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		*(pRealData + pit * y + x * bitCount + 0) = checkColorSpace(*(pRealData + pit * y + x * bitCount + 0)*KB);
		*(pRealData + pit * y + x * bitCount + 1) = checkColorSpace(*(pRealData + pit * y + x * bitCount + 1)*KG);
		*(pRealData + pit * y + x * bitCount + 2) = checkColorSpace(*(pRealData + pit * y + x * bitCount + 2)*KR);
	}

	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}
UINT ImageProcess::colorBalance(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	//图像的整个宽度和高度
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int shadow = param->colorBalanceParam->shadow;
	int highlight = param->colorBalanceParam->highlight;
	double midtones = param->colorBalanceParam->midtones;
	int outHightlight = param->colorBalanceParam->outHightlight;
	int outShadow = param->colorBalanceParam->outShadow;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	//GetBits() 获得数据区的指针
	byte* pRealData = (byte*)param->src->GetBits();
	//GetBPP() 返回每个像素所占的bit数，在这里/8意味着bitCount是每个像素所占的byte数
	int bitCount = param->src->GetBPP() / 8;
	//获得图像数据每一行的字节数
	//The pitch of the image.If the return value is negative, the bitmap is a bottom - up DIB and its origin is the lower left corner.
	//	If the return value is positive, the bitmap is a top - down DIB and its origin is the upper left corner.
	int pit = param->src->GetPitch();
	int diff = highlight - shadow;
	double rgbDiff, clRGB, outCIRGB;
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		for (int off = 0; off < bitCount; off++) {//bitCount = 1 意味着这是灰度图255位，若bitCount = 3 则意味着这是RGB（比如）图，将分别对3byte的数据一byte一byte赋值
			rgbDiff = *(pRealData + pit * y + x * bitCount + off) - shadow;
			//clRGB = pow(rgbDiff / diff, 1 / midtones);
			//outCIRGB = clRGB*(outHightlight - outShadow) / 255 + outShadow;
			//线性映射
			if (rgbDiff <= 0)
				outCIRGB = outShadow;
			else if(rgbDiff >= diff)
				outCIRGB = outHightlight;
			else outCIRGB = outShadow + rgbDiff / diff*(outHightlight - outShadow);
			*(pRealData + pit * y + x * bitCount + off) = checkColorSpace(outCIRGB);
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}
UINT ImageProcess::imageFusion(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	//图像的整个宽度和高度
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	//输入参数
	double alpha = param->alpha;

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	//GetBits() 获得数据区的指针
	byte* pRealData = (byte*)param->src->GetBits();
	byte* pOriginData = (byte*)param->origin_src->GetBits();//param->origin_src指的是第二张图片
	//GetBPP() 返回每个像素所占的bit数，在这里/8意味着bitCount是每个像素所占的byte数
	int bitCount = param->src->GetBPP() / 8;
	//获得图像数据每一行的字节数
	//The pitch of the image.If the return value is negative, the bitmap is a bottom - up DIB and its origin is the lower left corner.
	//	If the return value is positive, the bitmap is a top - down DIB and its origin is the upper left corner.
	int pit = param->src->GetPitch();
	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		double value;
		for (int off = 0; off < bitCount; off++) {//bitCount = 1 意味着这是灰度图255位，若bitCount = 3 则意味着这是RGB（比如）图，将分别对3byte的数据一byte一byte赋值
			double P1 = *(pRealData + pit * y + x * bitCount + off);
			double P2 = *(pOriginData + pit * y + x * bitCount + off);
			value = P1*alpha + (1-alpha)*P2;
			*(pRealData + pit * y + x * bitCount + off) = checkColorSpace(value);
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}
UINT ImageProcess::rotatePicture(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	//输入参数
	double theta = (param->angle) * DegreesToRadians;// 旋转的角度
	short type = 2;	//0 最近邻内插值 ;1 二阶线性插值;2 三阶插值

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	byte* pRealData = (byte*)param->src->GetBits();
	byte* pOriginData = (byte*)param->origin_src->GetBits();//param->origin_src指的是原图像，这里不会对它进行操作
	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();
	for (int i = startIndex; i <= endIndex; ++i)//扫描输出像素的位置，并在每一个位置（x,y）使用(v,w) = T^(-1)(x,y)计算输入图像中的相应位置
	{
		//x,y是像素的索引?
		int x = i % maxWidth;
		int y = i / maxWidth;

		//v,w是原图像中像素的坐标
		//int v = x*cos(theta) + y*sin(theta);
		//int w = (-1)*x*sin(theta) + y*cos(theta);
		//经过旋转后的v,w极有可能带有小数
		double real_v = x*cos(theta) + y*sin(theta) + cos(theta)*(-1)*(maxWidth / 2) + sin(theta)*(-1)*(maxHeight / 2) - (-1)*maxWidth / 2;
		double real_w = (-1)*x*sin(theta) + y*cos(theta) + cos(theta)*(-1)*(maxHeight / 2) - sin(theta)*(-1)*(maxWidth / 2) - (-1)*maxHeight / 2;
		//在每一个位置（x,y）使用(v,w) = T^(-1)(x,y)计算输入图像中的相应位置 
		double value = 0;
		int v, w;
		for (int off = 0; off < bitCount; off++) {//bitCount = 1 意味着这是灰度图255位，若bitCount = 3 则意味着这是RGB（比如）图，将分别对3byte的数据一byte一byte赋值
			if (type == 0) {//最近邻内插值
				//取最近的点 double转为int或许是直接舍去小数部分 这里不确定
				v = real_v;
				w = real_w;
				if ((value = normaliseXY(v, w, maxWidth, maxHeight)) != 0)//若没超出边界的话
				    value = *(pOriginData + pit * w + v * bitCount + off);
			}
			else if (type == 1) {//双线性插值 f(x , y) = f(X + u, Y + v) =f (X, Y)  * (1 - u) * (1 - v) + f(X, Y + 1) * (1 - u) * v + f(X + 1, Y) * u * (1 - v) + f (X + 1, Y + 1) * u * v;
				//取整数部分
				v = floor(real_v);
				w = floor(real_w);
				//设u与k分别为X + u，Y + k的小数部分
				double u = real_v - v;
				double k = real_w - w;
				if ((value = normaliseXY(v, w, maxWidth, maxHeight)) != 0)//若没超出边界的话
					if (v >= 1 && v < maxWidth - 1 && w >= 1 && w < maxHeight - 1) {//防止越界
						//正确实现二阶线性插值
						value = 0;
						value += (*(pOriginData + pit * (w)+(v)* bitCount + off))*(1 - u)*(1 - k);
						value += (*(pOriginData + pit * (w + 1) + (v)* bitCount + off))*(1 - u)*(k);
						value += (*(pOriginData + pit * (w)+(v + 1)* bitCount + off))*(u)*(1 - k);
						value += (*(pOriginData + pit * (w + 1) + (v + 1)* bitCount + off))*(u)*(k);
					}
			}
			else if (type == 2) {//双三次内插
				//取整数部分
				v = floor(real_v);
				w = floor(real_w);
				if ((value = normaliseXY(v, w, maxWidth, maxHeight)) != 0)//若没超出边界的话
					if (v >= 2 && v < maxWidth - 2 && w >= 2 && w < maxHeight - 2) {//防止越界														
						//实现双三次内插
						value = 0;
						//4*4领域点操作
						for(int i=-1;i<3;i++)
							for(int j=-1;j<3;j++)
								value += (*(pOriginData + pit * (w+i)+(v+j)* bitCount + off))*BiCubicPoly(real_v-v-j)*BiCubicPoly(real_w - w - i);
					}
			}
			value = checkColorSpace(value);
			*(pRealData + pit * y + x * bitCount + off) = value;
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}

UINT ImageProcess::zoomPicture(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	//输入参数
	double factor = param->zoom_factor;// 缩放的大小
	short type = 2;//0 最近邻内插值 ;1 二阶线性插值

	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();
	int origin_maxWidth = param->origin_src->GetWidth();
	int origin_maxHeight = param->origin_src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	byte* pRealData = (byte*)param->src->GetBits();
	byte* pOriginData = (byte*)param->origin_src->GetBits();//param->origin_src指的是原图像，这里不会对它进行操作
	int bitCount = param->src->GetBPP() / 8;
	int pit = param->src->GetPitch();
	int origin_pit = param->origin_src->GetPitch();

	for (int i = startIndex; i <= endIndex; ++i)//扫描输出像素的位置，并在每一个位置（x,y）使用(v,w) = T^(-1)(x,y)计算输入图像中的相应位置
	{
		//x,y是目标图像像素的索引
		int x = i % maxWidth;
		int y = i / maxWidth;
		//v,w是原图像中像素的坐标
		double real_v = x/factor ;
		double real_w = y/factor ;
		//在每一个位置（x,y）使用(v,w) = T^(-1)(x,y)计算输入图像中的相应位置 
		double value = 0;
		int v, w;
		for (int off = 0; off < bitCount; off++) {//RGB图后面的其他通道赋值
			if (type == 0) {
							//取最近的点 double转为int或许是直接舍去小数部分 这里不确定
				v = real_v;
				w = real_w;
				if ((value = normaliseXY(v, w, origin_maxWidth, origin_maxHeight)) != 0)//若没超出边界的话
					value = *(pOriginData + origin_pit * w + v * bitCount + off);
			}
			//最近邻内插值
			else if (type == 1) {
								 //取整数部分
				v = floor(real_v);
				w = floor(real_w);
				//设u与k分别为X + u，Y + k的小数部分
				double u = real_v - v;
				double k = real_w - w;
				if ((value = normaliseXY(v, w, origin_maxWidth, origin_maxHeight)) != 0)//若没超出边界的话
					if (v >= 1 && v < origin_maxWidth - 1 && w >= 1 && w < origin_maxHeight - 1) {//防止越界
																					//正确实现二阶线性插值
						value = 0;
						value += (*(pOriginData + origin_pit * (w)+(v)* bitCount + off))*(1 - u)*(1 - k);
						value += (*(pOriginData + origin_pit * (w + 1) + (v)* bitCount + off))*(1 - u)*(k);
						value += (*(pOriginData + origin_pit * (w)+(v + 1)* bitCount + off))*(u)*(1 - k);
						value += (*(pOriginData + origin_pit * (w + 1) + (v + 1)* bitCount + off))*(u)*(k);
					}
			}
			//双线性插值 f(x , y) = f(X + u, Y + v) =f (X, Y)  * (1 - u) * (1 - v) + f(X, Y + 1) * (1 - u) * v + f(X + 1, Y) * u * (1 - v) + f (X + 1, Y + 1) * u * v;
			else if (type == 2) {//双三次内插
								 //取整数部分
				v = floor(real_v);
				w = floor(real_w);
				if ((value = normaliseXY(v, w, origin_maxWidth, origin_maxHeight)) != 0)//若没超出边界的话
					if (v >= 2 && v < origin_maxWidth - 2 && w >= 2 && w < origin_maxHeight - 2) {//防止越界														
																					//实现双三次内插
						value = 0;
						//4*4领域点操作
						for (int i = -1; i<3; i++)
							for (int j = -1; j<3; j++)
								value += (*(pOriginData + origin_pit * (w + i) + (v + j)* bitCount + off))*BiCubicPoly(real_v - v - j)*BiCubicPoly(real_w - w - i);
					}
			}
			value = checkColorSpace(value);//避免value超过255或者低于0
			*(pRealData + pit * y + x * bitCount + off) = value;//设置图像的值
		}
	}
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}

UINT ImageProcess::addNoise(LPVOID  p)
{
	ThreadParam* param = (ThreadParam*)p;
	//图像的整个宽度和高度
	int maxWidth = param->src->GetWidth();
	int maxHeight = param->src->GetHeight();

	int startIndex = param->startIndex;
	int endIndex = param->endIndex;
	//GetBits() 获得数据区的指针
	byte* pRealData = (byte*)param->src->GetBits();
	//GetBPP() 返回每个像素所占的bit数，在这里/8意味着bitCount是每个像素所占的byte数
	int bitCount = param->src->GetBPP() / 8;
	//获得图像数据每一行的字节数
	//The pitch of the image.If the return value is negative, the bitmap is a bottom - up DIB and its origin is the lower left corner.
	//	If the return value is positive, the bitmap is a top - down DIB and its origin is the upper left corner.
	int pit = param->src->GetPitch();

	for (int i = startIndex; i <= endIndex; ++i)
	{
		int x = i % maxWidth;
		int y = i / maxWidth;
		if ((rand() % 1000) * 0.001 < NOISE)//随机事件：添加噪声
		{
			int value = 0;
			if (rand() % 1000 < 500)
			{
				value = 0;//黑点
			}
			else
			{
				value = 255;//白点
			}
			if (bitCount == 1)//若是255位彩色图像
			{
				*(pRealData + pit * y + x * bitCount) = value;
			}
			else//这里指的就是RGB 255*255*255真彩色
			{
				*(pRealData + pit * y + x * bitCount) = value;
				*(pRealData + pit * y + x * bitCount + 1) = value;
				*(pRealData + pit * y + x * bitCount + 2) = value;
			}
		}
	}
	//???
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), WM_NOISE, 1, NULL);
	return 0;
}

