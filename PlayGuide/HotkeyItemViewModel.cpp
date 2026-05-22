#include "pch.h"
#include "HotkeyItemViewModel.h"
#include "HotkeyItemViewModel.g.cpp"

namespace winrt::PlayGuide::implementation
{
    IObservableVector<hstring> HotkeyItemViewModel::SplitString(hstring const&key) noexcept
    {
        auto parts = winrt::single_threaded_observable_vector<winrt::hstring>();
        std::wstring str{ m_key };
        size_t start = 0, end;
        while ((end = str.find(L"+", start)) != std::wstring::npos) {
            parts.Append(str.substr(start, end - start).c_str());
            start = end + 1;
        }
        parts.Append(str.substr(start).c_str());
        return parts;
    }
}
