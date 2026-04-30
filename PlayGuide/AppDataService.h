#pragma once
#include "Appdata.h"
#include "IniHelper.h"
#include <unordered_map>
#include <string>

class AppDataService
{
public:
    static AppDataService& Get() {
        static AppDataService ins;
        return ins;
    }
    void Initialize(const std::wstring& path);

    Appdata LoadState();
    void SaveState(Appdata const& s);

    std::unordered_map<USHORT, UINT> GetHotkey() const;

    Appdata& State();
private:
    AppDataService() = default;
    ~AppDataService() {};

    AppDataService(const AppDataService&) = delete;
    AppDataService& operator=(const AppDataService&) = delete;

private:
    Appdata m_state;
    IniHelper m_ini{ L"config.ini" };
    std::unordered_map<USHORT, UINT> m_hotkey;
};