#pragma once
#include "Event.h"
#include "Appdata.h"
#include <string>
#include <winrt/Microsoft.UI.Xaml.Media.h>

struct TabInfoEx : TabInfo
{
    winrt::Microsoft::UI::Xaml::Media::ImageSource favicon{ nullptr };

    bool isPlayingAudio = false;
    bool isError = false;
};
inline Event<> g_processExitEvent;

inline Event<UINT> g_pipeServiceHandleEvent;
inline Event<> g_languageChanged;
inline Event<const std::wstring&> g_hotkeyChanged;

inline Event<const TabInfo&> g_newWindowRequested;
inline Event<const TabInfo&> g_navigationStarting;
inline Event<const TabInfo&> g_sourceChanged;
inline Event<const TabInfo&> g_documentTitleChanged;
inline Event<const TabInfo&> g_newUrlRequestEvent;
inline Event<const TabInfoEx&> g_faviconChanged;
inline Event<const TabInfoEx&> g_isDocumentPlayingAudio;