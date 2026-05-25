#pragma once
#include "Event.h"
#include "Appdata.h"
#include <string>

inline Event<> g_processExitEvent;

inline Event<UINT> g_pipeServiceHandleEvent;
inline Event<> g_languageChanged;
inline Event<const std::wstring&> g_hotkeyChanged;

inline Event<const TabInfo&> g_newWindowRequested;