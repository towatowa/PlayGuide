#pragma once
#include "Event.h"

inline Event<> g_processExitEvent;

inline Event<UINT> g_pipeServiceHandleEvent;
inline Event<> g_languageChanged;
inline Event<> g_hotkeyChanged;