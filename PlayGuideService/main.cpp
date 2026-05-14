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
#include "PipeService.h"
#include "RawInputManager.h"
#include <unordered_set>
#include <unordered_map>
#include "KeyStateTracker.h"
#include "workerqueue.h"


std::atomic<std::shared_ptr<std::unordered_set<HWND>>> g_filteredWindows{ 
    std::make_shared<std::unordered_set<HWND>>()
};
std::atomic<std::shared_ptr<std::unordered_map<Key, UINT>>> g_hotkeysMap{
    std::make_shared<std::unordered_map<Key, UINT>>()
};

std::atomic<std::shared_ptr<InputEvent>> g_inputHandle;

static void InputCallback(const SimpleEvent& ev)
{
    //记录按键
    KeyStateTracker::instance()->Update(ev);
    if (ev.action == KeyAction::KeyUp)
        return;
    HWND hwnd = nullptr;
    //获取窗口HWND
    hwnd = GetForegroundWindow();
    hwnd = GetAncestor(hwnd, GA_ROOT); // 获取顶层窗口

    //过滤窗口
    auto filters = g_filteredWindows.load();
    if (filters->find(hwnd) != filters->end())
        return;
    Key key(KeyStateTracker::instance()->GetCurrentModifiersKey(), ev.vk);
    LOG_DEBUG << "检测到按键事件: " << key.vk << " (HWND: " << hwnd << ")\n";
    auto map = g_hotkeysMap.load();
    auto it = map->find(key);
    if (it != map->end())
    {
        UINT msg = it->second;
        LOG_DEBUG << "触发热键消息: " << msg << "\n";
        PipeService::Get().SendHotkeyMsg(msg);
    }

}

int main()
{
    std::cout << "init..." << std::endl;
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::wcout.imbue(std::locale(""));

    //Logger::Instance().AddSink(std::make_unique<StdoutSink>());
#ifdef _DEBUG
    Logger::Instance().AddSink(std::make_unique<DebugOutputSink>());
#endif
    Logger::Instance().AddSink(std::make_unique<FileSink>((utils::GetExeDir()) / L"server_log.txt"));
    Logger::Instance().Start();
    LOG_INFO << "初始化日志成功\n";
    AppDataService::Get().Initialize((utils::GetExeDir()) / L"config.ini");

    //g_inputHandle.store(std::make_shared<RawInputManager>());
    g_hotkeysMap = std::make_shared<std::unordered_map<Key, UINT>>(AppDataService::Get().GetHotKeyCache());

    auto settings = AppDataService::Get().GetAppSettings();
    if (settings->inputType == InputType::KeyboardHook)
        g_inputHandle.store(std::make_shared<KeyboardHook>());
    else if (settings->inputType == InputType::RawInput)
        g_inputHandle.store(std::make_shared<RawInputManager>());

    //注册管道回调
    PipeService::Get().SetFilterRuleHandler(
        [](HWND hwnd)
        {
            auto oldSet = g_filteredWindows.load();

            auto newSet = std::make_shared<std::unordered_set<HWND>>(*oldSet);

            newSet->insert(hwnd);

            // ⭐ 原子替换
            g_filteredWindows.store(newSet);
            LOG_INFO << "添加过滤窗口: " << hwnd << "\n";
        });

    PipeService::Get().SetHotkeyEditHandler(
        [](UINT msg, Key key)
        {
            auto oldMap = g_hotkeysMap.load();

            auto newMap = std::make_shared<std::unordered_map<Key, UINT>>(*oldMap);

            // 删除旧映射（value == msg）
            for (auto it = newMap->begin(); it != newMap->end(); )
            {
                if (it->second == msg)
                    it = newMap->erase(it);
                else
                    ++it;
            }

            // 插入新键
            (*newMap)[key] = msg;

            // ⭐ 原子替换
            g_hotkeysMap.store(newMap);
            LOG_INFO << "更新热键映射: " << key.GetString() << " -> " << msg << "\n";
        });

    PipeService::Get().SetInputMethodHandler([](InputType inputType) {
        g_inputHandle.load()->Stop();
        
        if (inputType == InputType::KeyboardHook)
            g_inputHandle.store(std::make_shared<KeyboardHook>());
        else if (inputType == InputType::RawInput)
            g_inputHandle.store(std::make_shared<RawInputManager>());
        g_inputHandle.load()->SetCallback(&InputCallback);
        g_inputHandle.load()->Start();

        LOG_INFO << "切换输入监听方式: " << (inputType == InputType::KeyboardHook ? "KeyboardHook" : "RawInput") << "\n";
    });
    /*
    WorkerQueue<UINT>::Instance().SetHandler([](UINT msg) {
        LOG_INFO << "触发热键消息: " << msg << "\n";
        PipeService::Get().SendHotkeyMsg(msg);
        });
        */
    WorkerQueue<UINT>::Instance().Start();
    PipeService::Get().StartAsServer();//启动管道服务
    //启动输入监听
    {
        g_inputHandle.load()->SetCallback(&InputCallback);
        g_inputHandle.load()->Start();
    }
    HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    WaitForSingleObject(hEvent, INFINITE);
    return 0;
}
