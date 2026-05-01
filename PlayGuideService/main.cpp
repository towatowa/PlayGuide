// PlayGuideServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <iostream>
#include "KeyboardHook.h"
#include "utils.h"
#include "Logger.h"
#include "AppDataService.h"
#include "PipeServer.h"

int main()
{
    std::cout << "init..." << std::endl;
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::wcout.imbue(std::locale(""));

    Logger::Instance().AddSink(std::make_unique<StdoutSink>());
    Logger::Instance().AddSink(std::make_unique<FileSink>((utils::GetExeDir()) / L"server_log.txt"));
    Logger::Instance().Start();
    LOG_INFO << "初始化日志成功\n";
    AppDataService::Get().Initialize((utils::GetExeDir()) / L"config.ini");
    auto hook = std::make_unique<KeyboardHook>();
    auto hotkey = AppDataService::Get().GetHotKeyCache();
    PipeServer::Get().Start();
    hook->SetCallback([&](SimpleEvent& ev)
        {
            
            if (hotkey.find(ev.vk) != hotkey.end())
            {
                ev.vk = hotkey[ev.vk];
                LOG_INFO << "Post msg " << hotkey[ev.vk] << "\n";
                PipeServer::Get().Post(ev);
            }
        });
    hook->Start();

    HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    WaitForSingleObject(hEvent, INFINITE);
    return 0;
}
