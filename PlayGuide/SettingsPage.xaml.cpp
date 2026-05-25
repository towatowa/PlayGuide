#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif
#include "KeyMapping.h"
#include "utils.h"
#include "PipeService.h"
#include "Global.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::PlayGuide::implementation
{
    void SettingsPage::HotkeyFlyout_Opening(
        IInspectable const& sender,
        IInspectable const&)
    {
        auto flyout =
            sender.as<winrt::Microsoft::UI::Xaml::Controls::Flyout>();
        m_flyout = flyout;
        auto btn =
            m_flyout.Target().as<winrt::Microsoft::UI::Xaml::Controls::Button>();

        m_currentEditing = btn.DataContext().as<PlayGuide::HotkeyItemViewModel>();
        flyout.Content().as<FrameworkElement>().DataContext(m_currentEditing);

        LOG_INFO << "m_currentEditing.Name=" << m_currentEditing.Name().c_str() << "\n";
        // 清空旧录制
        m_currentEditing.RecordingKeys().Clear();

        DispatcherQueue().TryEnqueue([this]()
            {
                auto content = m_flyout.Content().as<StackPanel>();
                content.Focus(FocusState::Programmatic);
            });
    }

    void SettingsPage::HotkeyCaptureBorder_KeyDown(
        IInspectable const&,
        winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        using namespace winrt::Windows::System;

        e.Handled(true);

        auto key = e.Key();

        if (!m_currentEditing)
            return;

        auto keys = m_currentEditing.RecordingKeys();

        // 删除最后一个
        if (key == VirtualKey::Delete ||
            key == VirtualKey::Back)
        {
            if (keys.Size() > 0)
            {
                keys.RemoveAtEnd();
            }

            return;
        }

        std::wstring keyName = utils::StringToWstring(KeyMapping::VKToKeyName(static_cast<USHORT>(key)));

        // 防止重复
        for (auto const& k : keys)
        {
            if (k == keyName)
                return;
        }

        keys.Append(keyName);
    }

    void SettingsPage::SaveCapture_Click(
        IInspectable const&,
        RoutedEventArgs const&)
    {
        CommitHotkey();

        // 关闭 Flyout
        m_flyout.Hide();
    }

    fire_and_forget SettingsPage::CommitHotkey()
    {
        if (!m_currentEditing)
            co_return;

        std::wstring result;

        auto keys = m_currentEditing.RecordingKeys();

        for (uint32_t i = 0; i < keys.Size(); i++)
        {
            if (i > 0)
                result += L" + ";

            result += keys.GetAt(i);
        }
        Key key(m_currentEditing.Key());
        m_currentEditing.SetHotkey(hstring(result));
        g_hotkeyChanged.Invoke(m_currentEditing.id().c_str());
        //AppSettingsViewModel::Instance()->UpdateHotkey(m_currentEditing.id(), result.c_str());
        co_await resume_background();
        AppDataService::Get().SaveHotkey(m_currentEditing.id().c_str(), result);
        
        co_return;
    }

    void SettingsPage::CancelCapture_Click(
        IInspectable const&,
        RoutedEventArgs const&)
    {
        if (m_currentEditing)
        {
            m_currentEditing.RecordingKeys().Clear();
        }

        m_flyout.Hide();
    }
}
