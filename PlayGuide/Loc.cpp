#include "pch.h"
#include "Loc.h"
#include "Loc.g.cpp"
#include "LocalizationHelper.h"
#include <winrt/Windows.UI.Xaml.Interop.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::PlayGuide::implementation
{
    DependencyProperty Loc::TextProperty()
    {
        static DependencyProperty property =
            DependencyProperty::RegisterAttached(
                L"Text",
                xaml_typename<hstring>(),
                xaml_typename<winrt::PlayGuide::Loc>(),
                PropertyMetadata(nullptr, PropertyChangedCallback(&Loc::OnTextChanged)));

        return property;
    }

    void Loc::SetText(
        DependencyObject const& obj,
        hstring const& value)
    {
        obj.SetValue(TextProperty(), box_value(value));
    }

    hstring Loc::GetText(
        DependencyObject const& obj)
    {
        return unbox_value<hstring>(obj.GetValue(TextProperty()));
    }

    void Loc::OnTextChanged(
        DependencyObject const& d,
        DependencyPropertyChangedEventArgs const& e)
    {
        auto key = unbox_value<hstring>(e.NewValue());

        auto text = LocalizationHelper::Get().String(key);

        if (auto tb = d.try_as<Microsoft::UI::Xaml::Controls::TextBlock>())
        {
            tb.Text(text);
        }
        else if (auto btn = d.try_as<Microsoft::UI::Xaml::Controls::Button>())
        {
            btn.Content(box_value(text));
        }
    }

    void Loc::RefreshTree(DependencyObject const& root)
    {
        if (!root)
            return;

        // 1. 检查当前节点有没有 Loc.Text
        auto value = root.ReadLocalValue(TextProperty());

        if (value != DependencyProperty::UnsetValue())
        {
            auto key = unbox_value<hstring>(value);

            // 重新触发 OnTextChanged
            root.SetValue(TextProperty(), box_value(key));
        }

        // 2. 遍历子节点
        int count = Media::VisualTreeHelper::GetChildrenCount(root);

        for (int i = 0; i < count; ++i)
        {
            auto child = Media::VisualTreeHelper::GetChild(root, i);

            RefreshTree(child);
        }
    }
}