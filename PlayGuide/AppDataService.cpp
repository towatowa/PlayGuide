#include "AppDataService.h"

void AppDataService::Initialize(const std::wstring& path)
{
    m_ini = IniHelper(path);

    m_state.x = m_ini.ReadInt(L"Window", L"X", 100);
    m_state.y = m_ini.ReadInt(L"Window", L"Y", 100);
    m_state.width = m_ini.ReadInt(L"Window", L"Width", 1280);
    m_state.height = m_ini.ReadInt(L"Window", L"Height", 800);
    m_state.maximized = m_ini.ReadInt(L"Window", L"Maximized", 0) != 0;
    m_state.alpha = m_ini.ReadInt(L"Window", L"alpha", 255);

    m_state.play = m_ini.ReadInt(L"Hotkey", L"Play");
    m_state.SeekAdd = m_ini.ReadInt(L"Hotkey", L"SeekAdd");
    m_state.SeekDec = m_ini.ReadInt(L"Hotkey", L"SeekDec");
    m_state.show = m_ini.ReadInt(L"Hotkey", L"Show");
    m_state.OpacityAdd = m_ini.ReadInt(L"Hotkey", L"OpacityAdd");
    m_state.OpacityDec = m_ini.ReadInt(L"Hotkey", L"OpacityDec");

    m_hotkey[m_state.play] = WM_PLAY;
    m_hotkey[m_state.SeekAdd] = WM_SEEK_ADD;
    m_hotkey[m_state.SeekDec] = WM_SEEK_DEC;
    m_hotkey[m_state.show] = WM_SHOW_HIDE_WINDOW;
    m_hotkey[m_state.OpacityAdd] = WM_OPACITY_ADD;
    m_hotkey[m_state.OpacityDec] = WM_OPACITY_DEC;

    m_state.url = m_ini.ReadString(L"Web", L"Url", L"https://www.bing.com");
}

Appdata AppDataService::LoadState()
{
    return m_state;
}

void AppDataService::SaveState(Appdata const& s)
{
    m_ini.WriteInt(L"Window", L"X", s.x);
    m_ini.WriteInt(L"Window", L"Y", s.y);
    m_ini.WriteInt(L"Window", L"Width", s.width);
    m_ini.WriteInt(L"Window", L"Height", s.height);
    m_ini.WriteInt(L"Window", L"Maximized", s.maximized ? 1 : 0);
    m_ini.WriteInt(L"Window", L"alpha", s.alpha);

    m_ini.WriteInt(L"Hotkey", L"Play", s.play);
    m_ini.WriteInt(L"Hotkey", L"SeekAdd", s.SeekAdd);
    m_ini.WriteInt(L"Hotkey", L"SeekDec", s.SeekDec);
    m_ini.WriteInt(L"Hotkey", L"Show", s.show);
    m_ini.WriteInt(L"Hotkey", L"OpacityAdd", s.OpacityAdd);
    m_ini.WriteInt(L"Hotkey", L"OpacityDec", s.OpacityDec);

    m_ini.WriteString(L"Web", L"Url", s.url);
}

std::unordered_map<USHORT, UINT> AppDataService::GetHotkey() const
{
    return m_hotkey;
}

Appdata& AppDataService::State()
{
    return m_state;
}
