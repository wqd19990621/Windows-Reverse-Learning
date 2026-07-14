// MainDialog.cpp — MFC 扫雷辅助主对话框实现
//
// 整体思路：
//   1. 通过 FindWindow("扫雷") 定位游戏窗口
//   2. 使用 ReadProcessMemory 读取雷区布局、尺寸等关键数据
//   3. 通过 SendMessage 模拟鼠标点击实现自动扫雷

#include "pch.h"
#include "framework.h"
#include "Lenss.h"
#include "MainDialog.h"
#include "afxdialogex.h"


// ====================================================================
// 成员初始化
// ====================================================================

CMainDialog::CMainDialog(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_LENSS_DIALOG, pParent)
    , m_editbase(0)
    , m_strshowdata(_T(""))
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


// ====================================================================
// 数据交换 & 消息映射
// ====================================================================

void CMainDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT1, m_editbase);
    DDX_Text(pDX, IDC_EDIT2, m_strshowdata);
}

BEGIN_MESSAGE_MAP(CMainDialog, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CMainDialog::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CMainDialog::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON3, &CMainDialog::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_BUTTON4, &CMainDialog::OnBnClickedButton4)
    ON_BN_CLICKED(IDC_BUTTON5, &CMainDialog::OnBnClickedButton5)
END_MESSAGE_MAP()


// ====================================================================
// 内部辅助
// ====================================================================

/// @brief 确保扫雷窗口存在并已打开进程句柄
/// @return 成功返回 true；失败弹出错误并返回 false
bool CMainDialog::EnsureGameReady()
{
    if (m_reader.IsOpen())
        return true;

    if (!m_reader.Open(L"扫雷"))
    {
        ::MessageBox(nullptr, _T("扫雷游戏未打开"), _T("错误"), MB_OK);
        return false;
    }
    return true;
}


// ====================================================================
// 标准对话框消息（初始化、系统菜单、绘制、图标拖拽）
// ====================================================================

BOOL CMainDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        CString strAboutMenu;
        if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);
    return TRUE;
}

void CMainDialog::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMainDialog::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width()  - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

HCURSOR CMainDialog::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


// ====================================================================
// Button1 — 启动初级扫雷（发送 WM_COMMAND 0x209）
// ====================================================================

void CMainDialog::OnBnClickedButton1()
{
    HWND hwnd = ::FindWindow(nullptr, _T("扫雷"));
    if (!hwnd)
    {
        ::MessageBox(nullptr, _T("扫雷游戏未打开"), _T("错误"), MB_OK);
        return;
    }
    ::SetForegroundWindow(hwnd);
    Sleep(100);
    ::SendMessage(hwnd, WM_COMMAND, 0x209, 0);
}


// ====================================================================
// Button2 — 启动中级扫雷（发送 WM_COMMAND 0x20A）
// ====================================================================

void CMainDialog::OnBnClickedButton2()
{
    HWND hwnd = ::FindWindow(nullptr, _T("扫雷"));
    if (!hwnd)
    {
        ::MessageBox(nullptr, _T("扫雷游戏未打开"), _T("错误"), MB_OK);
        return;
    }
    ::SetForegroundWindow(hwnd);
    Sleep(100);
    ::SendMessage(hwnd, WM_COMMAND, 0x20A, 0);
}


// ====================================================================
// Button3 — 从固定地址读取 4 字节数据（测试 ReadProcessMemory）
// ====================================================================

void CMainDialog::OnBnClickedButton3()
{
    if (!EnsureGameReady())
        return;

    if (!m_reader.Read(0x1005194, &m_editbase, sizeof(m_editbase)))
    {
        ::MessageBox(nullptr, _T("进程内存读取失败"), _T("错误"), MB_OK);
        return;
    }
    UpdateData(FALSE);   // 将 m_editbase 显示到 IDC_EDIT1
}


// ====================================================================
// Button4 — 读取雷区数据并以十六进制显示到 Edit2
// ====================================================================
//
// 游戏数据布局（经典 winmine.exe x86）：
//   雷区数据位于 0x01005361，每行最多 32 字节
//   每个格子 1 字节：0x0F=未打开, 0x41=数字1, …, 0x8F=雷, 0x10=行尾
//   高 32 位数据存于 0x01005338，用于控制行遍历
//
//   ┌─────────────────────────────────────────────────┐
//   │  0x01005334 — 雷区宽度 (DWORD)                  │
//   │  0x01005338 — 雷区高度 (DWORD)                  │
//   │  0x01005361 — 雷区数据起始 (BYTE array[24][32]) │
//   │  0x01005194 — 基址测试标记 (DWORD)              │
//   └─────────────────────────────────────────────────┘

void CMainDialog::OnBnClickedButton4()
{
    if (!EnsureGameReady())
        return;

    // ---- 读取雷区数据 ----
    unsigned char gamedata[24][32] = {};
    if (!m_reader.Read(0x01005361, gamedata, sizeof(gamedata)))
    {
        ::MessageBox(nullptr, _T("读取扫雷游戏进程数据失败"), _T("错误"), MB_OK);
        return;
    }

    // ---- 读取高度 ----
    DWORD dwHeight = 0;
    if (!m_reader.Read(0x01005338, &dwHeight, sizeof(dwHeight)))
    {
        ::MessageBox(nullptr, _T("读取扫雷游戏进程数据失败"), _T("错误"), MB_OK);
        return;
    }

    // ---- 格式化输出 ----
    m_strshowdata.Empty();
    CString strTemp;
    for (DWORD i = 0; i < dwHeight; ++i)
    {
        for (int j = 0; j < 32; ++j)
        {
            if (gamedata[i][j] == 0x10) break;   // 行结束
            strTemp.Format(_T("%02X "), gamedata[i][j]);
            m_strshowdata += strTemp;
        }
        m_strshowdata += _T("\r\n");
    }

    UpdateData(FALSE);
}


// ====================================================================
// Button5 — 自动扫雷：读取雷区并点击所有非雷格子
// ====================================================================
//
// 原理：
//   1. 读取雷区数据，得到每个格子的状态
//   2. 标记值 0x8F 为雷，0x10 为行尾
//   3. 对其余格子通过 SendMessage 模拟鼠标左键点击
//   4. 坐标换算：startX/Y 为左上角偏移，cellSize=16 为格子边长
//
// ⚠️ 注意：坐标需根据实际扫雷窗口校准（startX/Y 在不同分辨率下可能不同）

void CMainDialog::OnBnClickedButton5()
{
    if (!EnsureGameReady())
        return;

    // ---- 读取雷区数据 ----
    unsigned char gamedata[24][32] = {};
    SIZE_T bytesRead = 0;
    if (!m_reader.Read(0x01005361, gamedata, sizeof(gamedata), &bytesRead))
    {
        ::MessageBox(nullptr, _T("读取雷区数据失败"), _T("错误"), MB_OK);
        return;
    }

    // ---- 读取高度 ----
    DWORD dwHeight = 0;
    if (!m_reader.Read(0x01005338, &dwHeight, sizeof(dwHeight), &bytesRead))
    {
        ::MessageBox(nullptr, _T("读取高度失败"), _T("错误"), MB_OK);
        return;
    }

    // ---- 激活扫雷窗口 ----
    HWND hwnd = m_reader.GetGameWindow();
    ::SetForegroundWindow(hwnd);
    ::BringWindowToTop(hwnd);
    Sleep(200);

    // ---- 坐标常量 ----
    const int startX   = 15;    // 第一个格子左上角 X
    const int startY   = 60;    // 第一个格子左上角 Y
    const int cellSize = 16;    // 每格像素边长

    // ---- 遍历并点击非雷格子 ----
    for (int row = 0; row < static_cast<int>(dwHeight); ++row)
    {
        for (int col = 0; col < 32; ++col)
        {
            BYTE cell = gamedata[row][col];
            if (cell == 0x10) break;    // 行结束
            if (cell == 0x8F) continue; // 跳过雷

            int x = startX + col * cellSize + cellSize / 2;
            int y = startY + row * cellSize + cellSize / 2;

            ::SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
            ::SendMessage(hwnd, WM_LBUTTONUP,   0,           MAKELPARAM(x, y));
            Sleep(10);   // 避免消息洪泛
        }
    }
}
