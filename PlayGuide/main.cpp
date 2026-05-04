#include "pch.h"
#include "App.xaml.h"
#include <winrt/base.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.Windows.AppLifecycle.h>
#include <utils.h>
#include "Logger.h"
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")
#include "PipeClient.h"
#include "AppDataService.h"
#include "WinEventMonitor.h"
#include <tlhelp32.h>
#include "Win32Helper.h"
#include "AppDataService.h"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::Windows::AppLifecycle;

#include <Windows.h>

HANDLE g_job = nullptr;

bool CreateProcessJob()
{
	g_job = CreateJobObjectW(nullptr, nullptr);
	if (!g_job)
		return false;

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION info{};
	ZeroMemory(&info, sizeof(info));

	info.BasicLimitInformation.LimitFlags =
		JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

	if (!SetInformationJobObject(
		g_job,
		JobObjectExtendedLimitInformation,
		&info,
		sizeof(info)))
	{
		CloseHandle(g_job);
		g_job = nullptr;
		return false;
	}

	return true;
}

bool StartAsAdmin(const std::wstring& exePath)
{
	SHELLEXECUTEINFOW sei{};
	sei.cbSize = sizeof(sei);

	sei.lpVerb = L"runas";
	sei.lpFile = exePath.c_str();
	sei.nShow = SW_HIDE;

	// ⭐关键：拿到 handle
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;

	bool ret = ShellExecuteExW(&sei);

	// 加入 Job
	AssignProcessToJobObject(g_job, sei.hProcess);

	CloseHandle(sei.hProcess);

	return ret;
}

// --------------------------
// 工具函数：强制杀死所有 WebView2 子进程（核心修复）
// --------------------------
void KillAllWebView2Processes()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) return;

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			// 杀死 WebView2 浏览器子进程
			if (_wcsicmp(pe32.szExeFile, L"msedgewebview2.exe") == 0)
			{
				HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
				if (hProcess)
				{
					TerminateProcess(hProcess, 0);
					CloseHandle(hProcess);
				}
			}
		} while (Process32Next(hSnapshot, &pe32));
	}
	CloseHandle(hSnapshot);
}

static void FixThreadPoolCrash() noexcept {
	assert(!GetModuleHandle(L"Windows.UI.Xaml.dll"));
	LoadLibraryEx(L"twinapi.appcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	LoadLibraryEx(L"threadpoolwinrt.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
}

int WINAPI wWinMain(
	HINSTANCE,
	HINSTANCE,
	PWSTR,
	int)
{
	/*
	const UINT32 majorMinorVersion{ 0x00010008 };
	PCWSTR versionTag{ L"" };
	const PACKAGE_VERSION minVersion{};
	const HRESULT hr{ MddBootstrapInitialize(majorMinorVersion, versionTag, minVersion) };
	*/
	// ==============================
   // ✔ 单例关键逻辑
   // ==============================
	auto mainInstance =
		AppInstance::FindOrRegisterForKey(L"PlayGuideMain");

	if (!mainInstance.IsCurrent())
	{
		// 把激活信息转发给主实例
		mainInstance.RedirectActivationToAsync(
			AppInstance::GetCurrent().GetActivatedEventArgs()
		).get();

		return 0; // 当前进程退出
	}

	FixThreadPoolCrash();
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	// 堆损坏时终止进程
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, nullptr, 0);

	Win32Helper::SetCurrentDirToExePath();
	if (!CreateProcessJob())
	{
		LOG_INFO << "创建 ProcessJob 失败，GetLastError=" << GetLastError() << "\n";
		return 0;
	}

	Logger::Instance().AddSink(std::make_unique<FileSink>(utils::GetExeDir() / L"log.txt"));
#ifdef _DEBUG
	Logger::Instance().AddSink(std::make_unique<DebugOutputSink>());
#endif
	Logger::Instance().Start();
	LOG_INFO << "初始化日志成功\n";
	
	if (!StartAsAdmin(utils::GetExeDir() / L"PlayGuideService.exe"))
	{
		LOG_INFO << "启动服务进程失败，GetLastError=" << GetLastError() << "\n";
		return 0;
	}
	
    AppDataService::Get().Initialize((utils::GetExeDir() / L"config.ini").wstring());
	LOG_INFO << "初始化应用数据成功\n";

	//启动事件监听线程
	PipeClient::Get().StartAsync();

	Application::Start(
		[&](auto&&)
		{
			FixThreadPoolCrash();
            static auto app = make<PlayGuide::implementation::App>();
		});
	
	//KillAllWebView2Processes();
	PipeClient::Get().Stop();
	Logger::Instance().Stop();
	// Release the DDLM and clean up.
	//MddBootstrapShutdown();
	return 0;
}