
// TreatHeadWriterDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CTreatHeadWriterDlg 对话框
class CTreatHeadWriterDlg : public CDialogEx
{
// 构造
public:
	CTreatHeadWriterDlg(CWnd* pParent = NULL);	// 标准构造函数

	static DWORD WINAPI ReadReport(void*);

// 对话框数据
	enum { IDD = IDD_TREATHEADWRITER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持



// 实现
protected:
	HICON m_hIcon;
	
	BOOL DeviceConnected;
	BOOL OnDeviceChange(UINT nEventType, DWORD dwData);
	void RefreshDevices(void);
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnAbout();
	afx_msg void OnCbnDropdownDevicelist();
	CComboBox m_cDeviceList;
	CButton m_connButton;
	afx_msg void OnBnClickedButton1();
	CStatic m_conn_stat;
	CEdit m_InputDebug;
};
