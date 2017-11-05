
// ExperimentImgDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "ImageProcess.h"


#define MAX_THREAD 8
#define MAX_SPAN 15
struct DrawPara
{
	CImage* pImgSrc;
	CDC* pDC;
	int oriX;
	int oriY;
	int width;
	int height;
};

// CExperimentImgDlg 对话框
class CExperimentImgDlg : public CDialogEx
{
// 构造
public:
	CExperimentImgDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXPERIMENTIMG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	CImage* getImage() { return m_pImgSrc; }
	void MedianFilter();
	void AddNoise();
	void WhiteBalance();
	void ImageFusion();
	void BilateralFilter();
	void ColorBalance();
	void RotateImage();
	void ZoomImage();
	void bilateralFilter(BilateralFilterParam * p);
	void AddNoise_WIN();
	void ThreadDraw(DrawPara *p);
	static UINT Update(void* p);
	void ImageCopy(CImage* pImgSrc, CImage* pImgDrt);
	void MedianFilter_WIN();
	void Rotate_WIN(double angle);
	void WhiteBalance_WIN();
	void ImageFusion_WIN(double alpha);
	void ColorBalance_WIN(ColorBalanceParam * p);
	void Zoom_WIN(double zoom_factor);
	void ImageFusion_BOOST(double alpha);
	LRESULT OnMedianFilterThreadMsgReceived(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNoiseThreadMsgReceived(WPARAM wParam, LPARAM lParam); 

// 实现
protected:
	HICON m_hIcon;
	CImage * m_pImgSrc;
	CImage * m_pImgCpy;
	int m_nThreadNum;
	ThreadParam* m_pThreadParam;
	CTime startTime;
	CString filePath;//一直保存着原始图像的路径，除非打开新的图片
//	ThreadParam * m_pThreadParam;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	void AutoSizePaint(CStatic & picture_Control, CImage * img);
	void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpen();
	CEdit mEditInfo;
	CStatic mPictureControl;
	afx_msg void OnCbnSelchangeComboFunction();
	afx_msg void OnNMCustomdrawSliderThreadnum(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderAlpha(NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnBnClickedButtonProcess();
	CButton m_CheckCirculation;
	CStatic originPictureControl;
	CEdit mOutputInfo;
	CEdit mEditAngle;
	CEdit m_shadow;
	CEdit m_highlight;
	afx_msg void OnBnClickedButtonReset();
	CEdit m_sigma_d;
};
