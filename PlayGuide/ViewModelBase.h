#pragma once
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/base.h>


struct ViewModelBase : winrt::implements<
    ViewModelBase,
    winrt::Microsoft::UI::Xaml::Data::INotifyPropertyChanged>
{
public:
    // 事件
    winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

    // INotifyPropertyChanged 实现
    winrt::event_token PropertyChanged(
        winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

protected:
    // 通知属性变化
    void RaisePropertyChanged(std::wstring const& propertyName)
    {
        m_propertyChanged(*this,
            winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs(propertyName));
    }

    // 通用 SetProperty（推荐用这个）
    template<typename T>
    bool SetProperty(T& storage, T const& value, std::wstring const& propertyName)
    {
        if (storage == value)
            return false;

        storage = value;
        RaisePropertyChanged(propertyName);
        return true;
    }
};
