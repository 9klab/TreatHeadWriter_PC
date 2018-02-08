
// TreatHeadWriterDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TreatHeadWriter.h"
#include "TreatHeadWriterDlg.h"
#include "afxdialogex.h"
#include <stdlib.h>
#include <windows.h>

//#include "INC\hidsdi++.h"
//#include "INC\hid.h"
#include <setupapi.h>
//#include "INC\usb100.h"
#include <hidsdi.h>
//#include <hid.h>

#include <dbt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BYTE InputReport[256];
BYTE OutputReport[256];
BOOL WRITE_ROPRT;
HANDLE        hDeviceHandle;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTreatHeadWriterDlg 对话框
GUID Guid;
HANDLE HidDeviceObject;
HIDP_CAPS Capabilities;
OVERLAPPED	HIDOverlapped;
DWORD NumberOfBytesRead;
DWORD NumberOfBytesWriten;

HANDLE ReadThread;
DWORD ReadThreadId;
HANDLE WriteThread;
DWORD WriteThreadId;
CArray<CString, CString&> DeviceListPatchArray;


CTreatHeadWriterDlg::CTreatHeadWriterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTreatHeadWriterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTreatHeadWriterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICELIST, m_cDeviceList);
	DDX_Control(pDX, IDC_BUTTON1, m_connButton);
	DDX_Control(pDX, IDC_CONN_STAT, m_conn_stat);
	DDX_Control(pDX, IDC_INPUTDEBUG, m_InputDebug);
}

BEGIN_MESSAGE_MAP(CTreatHeadWriterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_ABOUT, &CTreatHeadWriterDlg::OnAbout)
	ON_CBN_DROPDOWN(IDC_DEVICELIST, &CTreatHeadWriterDlg::OnCbnDropdownDevicelist)
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_BUTTON1, &CTreatHeadWriterDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CTreatHeadWriterDlg 消息处理程序

BOOL CTreatHeadWriterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	//ShowWindow(SW_MINIMIZE);

	// TODO:  在此添加额外的初始化代码
	CMenu menu;
	menu.LoadMenu(IDR_MENU1);  //IDR_MENU1为菜单栏ID号  
	SetMenu(&menu);
	
	DEV_BROADCAST_DEVICEINTERFACE filter = { 0 };
	filter.dbcc_size = sizeof(filter);
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	HidD_GetHidGuid(&Guid);
	filter.dbcc_classguid = Guid;
	RegisterDeviceNotification(GetSafeHwnd(), (PVOID)&filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	//if (ReadThread == NULL)
	//{
	//	ReadThread = CreateThread(NULL,
	//		0,
	//		ReadReport,
	//		m_hWnd,
	//		0,
	//		&ReadThreadId);
	//}

	DeviceConnected = 0;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}
BOOL CTreatHeadWriterDlg::OnDeviceChange(UINT nEventType, DWORD dwData)
{
	_DEV_BROADCAST_HEADER *hdr;
	switch (nEventType)
	{
	case DBT_DEVICEREMOVECOMPLETE://(nEventType == 0x0007))
		if (DeviceConnected == 1)
		{
			CloseHandle(hDeviceHandle);
		}

	case DBT_DEVICEARRIVAL:
		if (DeviceConnected == 0)
		{
			hdr = (_DEV_BROADCAST_HEADER*)dwData;
			if (hdr->dbcd_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
			{
				RefreshDevices();
			}
		}
		break;
	default:
		break;
	}
	return TRUE;
}
void CTreatHeadWriterDlg::RefreshDevices()
{
	WCHAR	Product[253];
	CHAR Prod[253];
	CString	 patch;
	DWORD k;

	m_cDeviceList.ResetContent();
	m_connButton.SetWindowTextA("连接");
	m_conn_stat.SetWindowTextA("已断开");
	HDEVINFO info;
	info = SetupDiGetClassDevs(&Guid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if (info != INVALID_HANDLE_VALUE)
	{
		
		DWORD devIndex;
		SP_INTERFACE_DEVICE_DATA ifData;
		ifData.cbSize = sizeof(ifData);

		for (devIndex = 0; SetupDiEnumDeviceInterfaces(info, NULL, &Guid, devIndex, &ifData); ++devIndex)
		{
			DWORD needed;

			SetupDiGetDeviceInterfaceDetail(info, &ifData, NULL, 0, &needed, NULL);

			PSP_INTERFACE_DEVICE_DETAIL_DATA detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)new BYTE[needed];
			detail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
			SP_DEVINFO_DATA did = { sizeof(SP_DEVINFO_DATA) };
			//SetupDiGetDeviceInterfaceDetail(info, &ifData, detail, needed, NULL, &did);
			//if (SetupDiGetDeviceRegistryProperty(info, &did, SPDRP_DEVICEDESC, NULL, (PBYTE)Product, 253, NULL))
			if (SetupDiGetDeviceInterfaceDetail(info, &ifData, detail, needed, NULL, &did))
			{

				// Add the link to the list of all HID devices
				if (strstr(detail->DevicePath, "vid_0483&pid_5751") != NULL)
				{
					patch = detail->DevicePath;
					hDeviceHandle = CreateFile(detail->DevicePath,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);
					if (hDeviceHandle == INVALID_HANDLE_VALUE)
						return;
					HidD_GetProductString(hDeviceHandle, Product, sizeof(Product));
					size_t t;
					wcstombs_s(&t,Prod,253, Product, 256);
					m_cDeviceList.AddString(Prod);
					DeviceListPatchArray.Add(patch);

					CloseHandle(hDeviceHandle);
				}
			}
			else
				m_cDeviceList.AddString("");
			delete[](PBYTE)detail;
		}
		SetupDiDestroyDeviceInfoList(info);
	}
	m_cDeviceList.SetCurSel(0);
	if (m_cDeviceList.GetCount() == 1)
	{
		hDeviceHandle = CreateFile(patch,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,//FILE_FLAG_OVERLAPPED,
			NULL);
		if (hDeviceHandle == INVALID_HANDLE_VALUE)
			return;
		PHIDD_ATTRIBUTES Attributes = (PHIDD_ATTRIBUTES)malloc(sizeof(HIDD_ATTRIBUTES));
		if (!HidD_GetAttributes(hDeviceHandle, Attributes))
		{
			CloseHandle(hDeviceHandle);
			//SetupDiDestroyDeviceInfoList(hDevInfo);
			return;
		}
		m_conn_stat.SetWindowTextA("已连接");
		m_connButton.SetWindowTextA("断开");
		GetDlgItem(IDC_DEVICELIST)->EnableWindow(false);
		DeviceConnected = 1;
		BOOL Result = ReadFile
			(hDeviceHandle,
			&InputReport,
			65,
			&NumberOfBytesRead,
			(LPOVERLAPPED)&HIDOverlapped);
		if (Result == TRUE && NumberOfBytesRead>0)
		{
			CString t, s;
			t.Format("0x%08X", *(DWORD*)InputReport);
			m_InputDebug.GetWindowTextA(s);
			s += t;
			m_InputDebug.SetWindowTextA(s);
		}
		else
		{
			k = GetLastError();
		}
	}
}
void CTreatHeadWriterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTreatHeadWriterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTreatHeadWriterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTreatHeadWriterDlg::OnAbout()
{
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();
}


void CTreatHeadWriterDlg::OnCbnDropdownDevicelist()
{
	//struct libusb_device_descriptor desc;
	//libusb_device **devs;
	//libusb_device_handle *handle = NULL;
	//char description[256];
	//BYTE string[256];
	//int ret, i,cnt;


	//libusb_init(NULL);
	//cnt = libusb_get_device_list(NULL, &devs);
	//for (int i = 0; i < cnt; cnt++)
	//{

	//	ret = libusb_get_device_descriptor(devs[i], &desc);
	//	if (ret < 0)
	//		continue;
	//	ret = libusb_open(devs[i], &handle);
	//	if (LIBUSB_SUCCESS == ret) {
	//		if (desc.iManufacturer) {
	//			ret = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, string, sizeof(string));
	//			if (ret > 0);
	//		}
	//		if (desc.iProduct) {
	//			ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, string, sizeof(string));
	//			if (ret > 0);
	//		}
	//	}
	//}
	//
}
DWORD WINAPI CTreatHeadWriterDlg::ReadReport(void* pData)
{
	CTreatHeadWriterDlg pDialog = static_cast< CTreatHeadWriterDlg* >(
		CWnd::FromHandle(reinterpret_cast< HWND >(pData)));
	//ASSERT_VALID(pDialog);
	while (TRUE)
	{
		if (!WRITE_ROPRT)
		{
			if (hDeviceHandle != INVALID_HANDLE_VALUE)
			{
				CString s,t;
				CancelIo(hDeviceHandle);
				//InputReport[0]=0;
				BOOL Result = HidD_GetInputReport
					(hDeviceHandle,
					&InputReport,
					64);
				if (Result == TRUE && NumberOfBytesRead>0)
				{
					t.Format("0x%08X", InputReport);
					//pDialog.m_InputDebug.GetWindowTextA(s);
					s += t;
				    //pDialog.m_InputDebug.SetWindowTextA(s);
				}
			}
		}
		Sleep(25);
	}

}

void CTreatHeadWriterDlg::OnBnClickedButton1()
{
	if (DeviceConnected == 1)
	{
		if (hDeviceHandle != INVALID_HANDLE_VALUE)
			CloseHandle(hDeviceHandle);
		m_conn_stat.SetWindowTextA("已断开");
		m_connButton.SetWindowTextA("连接");
		GetDlgItem(IDC_DEVICELIST)->EnableWindow(true);
		DeviceConnected = 0;
	}
	else
	{
		if (m_cDeviceList.GetCount() > 0 )
		{
			CString str = DeviceListPatchArray.GetAt(m_cDeviceList.GetCurSel());
				hDeviceHandle = CreateFile(str,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
			if (hDeviceHandle == INVALID_HANDLE_VALUE)
				return;
			m_conn_stat.SetWindowTextA("已连接");
			m_connButton.SetWindowTextA("断开");
			GetDlgItem(IDC_DEVICELIST)->EnableWindow(false);
			DeviceConnected = 1;

		}
	}
}
