#pragma once


// CDlgPadConfig ダイアログ

class CDlgPadConfig : public CDialog
{
	DECLARE_DYNAMIC(CDlgPadConfig)

public:
	CDlgPadConfig(CWnd* pParent = NULL);   // 標準コンストラクタ
	virtual ~CDlgPadConfig();

// ダイアログ データ
	enum { IDD = IDD_DIALOG_PAD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	PADCONFIG m_cfg;
	int m_crnt_pad;

	int m_padindex;
	int m_buttonA;
	int m_buttonB;
	int m_buttonC;
	int m_buttonD;
	int m_buttonSTART;
	int m_buttonA_B;
	int m_buttonB_C;
	int m_buttonC_A;
	int m_buttonA_B_C;
	afx_msg void OnCbnSelchangeComboPadindex();
protected:
	virtual void OnCancel();
};
