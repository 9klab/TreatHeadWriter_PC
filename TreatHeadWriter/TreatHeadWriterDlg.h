
// TreatHeadWriterDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


// CTreatHeadWriterDlg �Ի���
class CTreatHeadWriterDlg : public CDialogEx
{
// ����
public:
	CTreatHeadWriterDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TREATHEADWRITER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	BOOL OnDeviceChange(UINT nEventType, DWORD dwData);
	void RefreshDevices(void);
	// ���ɵ���Ϣӳ�亯��
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
};
