#include "pch.h"
#include "AboutWindow.xaml.h"
#if __has_include("AboutWindow.g.cpp")
#include "AboutWindow.g.cpp"
#endif
#include "Win32Helper.h"
#include <winrt/Microsoft.Windows.System.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.ApplicationModel.DataTransfer.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/base.h>
#include <WebView2.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.System.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Windows::ApplicationModel::DataTransfer;
using namespace Windows::Web::Http;
using namespace Windows::Data::Json;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::PlayGuide::implementation
{
    void AboutWindow::InitializeInfo()
    {

        VersionText().Text(Win32Helper::GetAppVersion().c_str());
        LPWSTR version = nullptr;

        GetAvailableCoreWebView2BrowserVersionString(nullptr, &version);
        WebView2VersionText().Text(version);
        CoTaskMemFree(version);
    }

    void AboutWindow::OnOpenGithub(
        IInspectable const&,
        RoutedEventArgs const&)
    {
        Windows::System::Launcher::LaunchUriAsync(
            Windows::Foundation::Uri(L"https://github.com/towatowa/PlayGuide"));
    }

    Windows::Foundation::IAsyncAction AboutWindow::OnCheckUpdate(
        IInspectable const&,
        RoutedEventArgs const&)
    {
        UpdateButton().Content(box_value(LocalizationHelper::Get().String(L"Checking")));

        try
        {
            HttpClient client;

            client.DefaultRequestHeaders().UserAgent().TryParseAdd(
                L"PlayGuide");

            auto response = co_await client.GetAsync(
                Windows::Foundation::Uri(L"https://api.github.com/repos/towatowa/PlayGuide/releases/latest"));

            auto jsonText = co_await response.Content().ReadAsStringAsync();

            auto json = JsonObject::Parse(jsonText);

            auto latestVersion =
                json.GetNamedString(L"tag_name");

            auto releaseUrl =
                json.GetNamedString(L"html_url");

            // 当前版本
            auto currentVersion =
                winrt::to_hstring(Win32Helper::GetAppVersion().c_str());

            // 去掉 v 前缀
            std::wstring latest = latestVersion.c_str();

            if (!latest.empty() && latest[0] == L'v')
            {
                latest.erase(0, 1);
            }

            bool hasUpdate = latest != currentVersion.c_str();

            DispatcherQueue().TryEnqueue([=]
                {
                    if (hasUpdate)
                    {
                        UpdateButton().Content(
                            box_value(L"New Version Available"));

                        // 打开 Release 页面
                        Windows::System::Launcher::LaunchUriAsync(
                            Windows::Foundation::Uri(releaseUrl));
                    }
                    else
                    {
                        UpdateButton().Content(
                            box_value(LocalizationHelper::Get().String(
                                L"AreadyLatest")));
                    }
                });
        }
        catch (...)
        {
            DispatcherQueue().TryEnqueue([this]
                {
                    UpdateButton().Content(
                        box_value(L"Check Failed"));
                });
        }

    }

    void AboutWindow::OnCopyInfo(
        IInspectable const&,
        RoutedEventArgs const&)
    {
        DataPackage package;

        hstring text =
            L"PlayGuide\n"
            L"Version: " + VersionText().Text() +
            L"\nWebView2: " + WebView2VersionText().Text();

        Clipboard::SetContent(package);
    }

    void AboutWindow::OnOpenLicense(IInspectable const&, RoutedEventArgs const&)
    {
            Windows::System::Launcher::LaunchUriAsync(
                Windows::Foundation::Uri(L"https://github.com/towatowa/PlayGuide?tab=MIT-1-ov-file"));
    }
}
