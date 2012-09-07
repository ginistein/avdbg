#pragma once


// CFileOpenDlg 对话框

class CFileOpenDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileOpenDlg)

public:
	CFileOpenDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileOpenDlg();

// 对话框数据
	enum { IDD = IDD_DIALOGOPEN };

	CString	m_strPath;
	CString	m_strParam;
	CString m_strRunDir;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonbrowse();
};
