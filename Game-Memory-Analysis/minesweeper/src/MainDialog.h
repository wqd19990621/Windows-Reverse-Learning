// MainDialog.h — MFC 扫雷辅助主对话框头文件
//
// 控件映射：
//   Button1 (IDC_BUTTON1) → 发送 WM_COMMAND 0x209 启动初级游戏
//   Button2 (IDC_BUTTON2) → 发送 WM_COMMAND 0x20A 启动中级游戏
//   Button3 (IDC_BUTTON3) → 读取基地址 0x1005194 并显示到 Edit1
//   Button4 (IDC_BUTTON4) → 读取雷区数据并以十六进制显示到 Edit2
//   Button5 (IDC_BUTTON5) → 自动读取雷区并点击所有非雷格子（自动扫雷）
//
// 内存偏移（适用于经典 Win7 扫雷 winmine.exe）：
//   雷区基址  = 0x01005361    宽 = 0x01005334    高 = 0x01005338
//   数据标记  = 0x10（行尾）  雷 = 0x8F

#pragma once
#include "MemoryReader.h"


/// @brief 扫雷辅助 MFC 对话框
class CMainDialog : public CDialogEx
{
public:
    CMainDialog(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_LENSS_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    // ---- 按钮事件 ----
    afx_msg void OnBnClickedButton1();   // 启动初级
    afx_msg void OnBnClickedButton2();   // 启动中级
    afx_msg void OnBnClickedButton3();   // 读取内存测试
    afx_msg void OnBnClickedButton4();   // 读取并显示雷区
    afx_msg void OnBnClickedButton5();   // 自动扫雷

    // ---- 控件绑定 ----
    int      m_editbase;          // 绑定到 IDC_EDIT1
    CString  m_strshowdata;       // 绑定到 IDC_EDIT2

private:
    HICON        m_hIcon;
    MemoryReader m_reader;        // 内存读取器封装

    // ---- 内部辅助 ----
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();

    /// @brief 定位扫雷窗口并打开进程句柄（内部快捷方法）
    bool EnsureGameReady();
};
