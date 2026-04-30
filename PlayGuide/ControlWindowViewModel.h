#pragma once
#include "ViewModelBase.h"
#include "ControlWindowViewModel.g.h"
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <string>

using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Data;

namespace winrt::PlayGuide::implementation {

	struct ControlWindowViewModel : ControlWindowViewModelT<ControlWindowViewModel, ViewModelBase>
	{
	public:
		void Title(hstring const& value) {
			if (value.empty() || m_title == value)
				return;
			SetProperty(m_title, value, L"Title");
		}
		hstring Title() noexcept {
			return m_title;
		}
	private:
		hstring m_title{ L"" };
	};
}

