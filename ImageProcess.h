#pragma once
#pragma once
#define NOISE 0.2
//  Define M_PI in the case it's not defined in the math header file
#ifndef M_PI
#  define M_PI  3.14159265358979323846
#endif
//  Degrees-to-radians constant 
const double  DegreesToRadians = M_PI / 180.0;
struct ThreadParam
{
	CImage * src;
	CImage * origin_src;//不做改动的图像，在某些变换中会用到
	int startIndex;
	int endIndex;
	int maxSpan;//为模板中心到边缘的距离
};

static bool GetValue(int p[], int size, int &value);

class ImageProcess
{
public:
	static UINT medianFilter(LPVOID  param);
	static UINT addNoise(LPVOID param);
	static UINT rotatePicture(LPVOID p);
	static UINT zoomPicture(LPVOID p);
};