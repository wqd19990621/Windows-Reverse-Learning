// MemoryReader.h — 封装 Windows 进程内存读取，专用于扫雷辅助
//
// 职责：
//   - 通过窗口标题 "扫雷" 定位扫雷进程
//   - 打开进程句柄并读取指定内存地址的数据
//   - 关闭进程句柄（RAII 封装）
//
// 典型用法：
//   MemoryReader reader;
//   if (reader.Open(L"扫雷")) {
//       DWORD height;
//       reader.Read(0x01005338, &height, sizeof(height));
//       reader.Close();
//   }

#pragma once

#include <Windows.h>
#include <string>

/// @brief 扫雷进程内存读取器
///
/// 封装 FindWindow + OpenProcess + ReadProcessMemory 流程，
/// 提供类型安全的内存读取接口。
class MemoryReader
{
public:
    MemoryReader() noexcept;
    ~MemoryReader() noexcept;

    // 禁止拷贝（句柄独享）
    MemoryReader(const MemoryReader&) = delete;
    MemoryReader& operator=(const MemoryReader&) = delete;

    // 允许移动
    MemoryReader(MemoryReader&& other) noexcept;
    MemoryReader& operator=(MemoryReader&& other) noexcept;

    /// @brief 通过窗口标题打开扫雷进程
    /// @param windowTitle 窗口标题，默认 "扫雷"
    /// @return 成功返回 true，失败返回 false
    bool Open(const wchar_t* windowTitle = L"扫雷");

    /// @brief 关闭进程句柄并重置状态
    void Close() noexcept;

    /// @brief 读取进程内存
    /// @param address  目标内存地址
    /// @param buffer   接收缓冲区（调用者保证大小足够）
    /// @param size     要读取的字节数
    /// @param bytesRead [可选] 实际读取的字节数
    /// @return 成功返回 true
    template<typename T>
    bool Read(uintptr_t address, T* buffer, SIZE_T size, SIZE_T* bytesRead = nullptr)
    {
        if (!m_hProcess)
            return false;

        return ::ReadProcessMemory(
            m_hProcess,
            reinterpret_cast<LPCVOID>(address),
            buffer,
            size,
            bytesRead ? bytesRead : &m_dummyBytesRead
        ) != FALSE;
    }

    /// @brief 判断进程句柄是否有效
    bool IsOpen() const noexcept { return m_hProcess != nullptr; }

    /// @brief 获取扫雷窗口句柄
    HWND GetGameWindow() const noexcept { return m_hWnd; }

private:
    HANDLE  m_hProcess;
    HWND    m_hWnd;
    SIZE_T  m_dummyBytesRead;   // 默认 bytesRead 接收者
};
