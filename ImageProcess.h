#pragma once
#pragma once
#define NOISE 0.2
//  Define M_PI in the case it's not defined in the math header file
#ifndef M_PI
#  define M_PI  3.14159265358979323846
#endif
//  Degrees-to-radians constant 
const double  DegreesToRadians = M_PI / 180.0;
//自动色阶均衡所用参数
struct ColorBalanceParam {
	int shadow;//输入黑场
	int highlight;//输入白场
	double midtones;//输入灰场
	int outHightlight;//输出白场
	int outShadow;//输出黑场
};
struct BilateralFilterParam {
	//mask的长度
	int filter_length;
	double sigma_d ;
	double sigma_r ;
};
struct ThreadParam
{
	CImage * src;
	CImage * origin_src;//不做改动的图像，在某些变换中会用到
	int startIndex;
	int endIndex;
	int maxSpan;//为模板中心到边缘的距离
	double angle;//旋转的角度
	double zoom_factor;//缩放因子
	double alpha;//合成的时候的alpha值
	double R_sum;//R通道分量总和
	double G_sum;
	double B_sum;
	ColorBalanceParam * colorBalanceParam;
	BilateralFilterParam * bilateralFilterParam;
};

static bool GetValue(int p[], int size, int &value);
int normaliseXY(int x, int y, int max_x, int max_y);
float BiCubicPoly(float x);
int checkColorSpace(double x);

class ImageProcess
{
public:
	static UINT medianFilter(LPVOID  param);
	static UINT bilateralFilter(LPVOID p);
	static UINT addNoise(LPVOID param);
	static UINT whiteBalance(LPVOID p);
	static UINT colorBalance(LPVOID p);
	static UINT imageFusion(LPVOID p);
	static UINT rotatePicture(LPVOID p);
	static UINT zoomPicture(LPVOID p);
};