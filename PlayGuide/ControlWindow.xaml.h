#pragma once

#if __has_include("ControlWindow.g.h")
#include "ControlWindow.g.h"
#endif
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "Event.h"
#include "Appdata.h"
//#include "PipeClient.h"
#include "AppDataService.h"

using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Dispatching;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::PlayGuide::implementation
{
    // 吸附阈值、展开/收缩尺寸
    constexpr int AdsorbThreshold = 15;
    constexpr int ExpandWidth = 854;
    constexpr int ExpandHeight = 120;
    constexpr int DockSnapWidth = 4;
    constexpr int DockSnapHeight = 4;

    // 停靠方向
    enum class DockSide
    {
        None,
        Left,
        Right,
        Top
    };

    struct ControlWindow : ControlWindowT<ControlWindow>
    {
        ControlWindow();

        void InitializeControl(HWND hwnd);
        
        RectInt32 GetScreenWorkArea() noexcept {
            auto windowId = winrt::Microsoft::UI::GetWindowIdFromWindow(m_hwnd);
            auto displayArea = DisplayArea::GetFromWindowId(
                windowId,
                DisplayAreaFallback::Nearest
            );
            return displayArea.WorkArea();
        };
        RectInt32 GetWindowRect() noexcept {

            auto position = AppWindow().Position();
            auto size = AppWindow().Size();

            return { position.X, position.Y, size.Width, size.Height };
        }
        // 判断当前窗口是否靠左/靠右/靠上边缘
        DockSide CheckDockSide(RectInt32 const& winBounds, RectInt32 const& screen) noexcept;
        void Grid_PointerPressed(IInspectable const&, PointerRoutedEventArgs const& e);
        void Grid_PointerMoved(IInspectable const&, PointerRoutedEventArgs const& e);
        void Grid_PointerReleased(IInspectable const&, PointerRoutedEventArgs const& e);
        void Grid_PointerEntered(IInspectable const&, PointerRoutedEventArgs const&);
        void Grid_PointerExited(IInspectable const&, PointerRoutedEventArgs const&);
       
        bool IsPointInsideWindow(POINT &pt);

        void SetVisibleInvoker(Event<bool>& event);

        void SetCloseEventInvoker(Event<bool>& event);
        void SetPageCreatedStateEventRevoker(Event<TabInfo>& event);
        void HandleEvent(UINT msg) noexcept;

        void ApplyWindowState(const ControlWindowData& state) noexcept;

        void SaveWindowStateData() noexcept;

        TabViewItem FindTabViewItem(uint32_t idx);

        void SetPipeServiceHandleEvent(Event<UINT>& event);

        Event<bool>::EventRevoker visibleInvoker;
        Event<bool>::EventRevoker closeEventRevoker;
        Event<int> tabSeletedChangedEvent;
        Event<int> tabCloseEvent;
        Event<TabInfo>newUrlEnterEvent;
        //Event<bool>::EventRevoker tabClosingStateEventRevoker;
        Event<TabInfo>::EventRevoker pageCreatedStateEventRevoker;
        Event<UINT>::EventRevoker m_pipeServiceHandleRevoker;
    private:
        // 拖拽状态
        bool        m_isDragging{ false };
        POINT       m_dragStartCursor{};
        PointInt32  m_dragStartWindowPos{};
        HWND        m_hwnd{ nullptr };
       
        RectInt32   m_screenCache{};
        Point       m_pendingMouse{};
        int         m_refreshRate{ 60 };
        
        bool        m_isActive{ false };
        bool        m_isEntered{ false };
        bool        m_userInteracted{ false };
        bool        m_isClosingTab{ false }; // 标记：正在关闭标签
        bool        m_isCreatingTab{ false };
        bool        m_pendingResize{ false };
        DispatcherQueueTimer m_hoverTimer{ nullptr }; 
        std::chrono::steady_clock::time_point m_lastMoveTime;
        hstring m_title;
        std::mutex m_mutex;
        uint32_t m_nextId{ 0 };
    };
}

namespace winrt::PlayGuide::factory_implementation
{
    struct ControlWindow : ControlWindowT<ControlWindow, implementation::ControlWindow>
    {
    };
}