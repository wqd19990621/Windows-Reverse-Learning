// MemoryReader.cpp — 进程内存读取器实现

#include "MemoryReader.h"
#include <TlHelp32.h>

// ---- 构造 / 析构 --------------------------------------------------------

MemoryReader::MemoryReader() noexcept
    : m_hProcess(nullptr)
    , m_hWnd(nullptr)
    , m_dummyBytesRead(0)
{
}

MemoryReader::~MemoryReader() noexcept
{
    Close();
}

MemoryReader::MemoryReader(MemoryReader&& other) noexcept
    : m_hProcess(other.m_hProcess)
    , m_hWnd(other.m_hWnd)
    , m_dummyBytesRead(other.m_dummyBytesRead)
{
    other.m_hProcess = nullptr;
    other.m_hWnd     = nullptr;
    other.m_dummyBytesRead = 0;
}

MemoryReader& MemoryReader::operator=(MemoryReader&& other) noexcept
{
    if (this != &other)
    {
        Close();
        m_hProcess       = other.m_hProcess;
        m_hWnd           = other.m_hWnd;
        m_dummyBytesRead = other.m_dummyBytesRead;
        other.m_hProcess = nullptr;
        other.m_hWnd     = nullptr;
        other.m_dummyBytesRead = 0;
    }
    return *this;
}

// ---- 公开方法 -----------------------------------------------------------

bool MemoryReader::Open(const wchar_t* windowTitle)
{
    // 1. 查找扫雷窗口
    m_hWnd = ::FindWindowW(nullptr, windowTitle);
    if (!m_hWnd)
        return false;

    // 2. 获取进程 ID
    DWORD pid = 0;
    ::GetWindowThreadProcessId(m_hWnd, &pid);
    if (pid == 0)
        return false;

    // 3. 打开进程（请求全部权限以便读写内存）
    m_hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    return m_hProcess != nullptr;
}

void MemoryReader::Close() noexcept
{
    if (m_hProcess)
    {
        ::CloseHandle(m_hProcess);
        m_hProcess = nullptr;
    }
    m_hWnd = nullptr;
}
