#pragma once

#include <string>

#include "imgui.h"
#include "fmt/format.h"


namespace ImGui
{
	template <typename... Args>
	void TextFmt(fmt::format_string<Args...> fmt, Args&&... args)
	{
		const std::string str = fmt::format(fmt, std::forward<Args>(args)...);
		TextUnformatted(&str.front(), (&str.back()) + 1);
	}

	inline void TreePop(u32& currentDepth, u32 targetDepth)
	{
		while (currentDepth > targetDepth)
		{
			TreePop();
			currentDepth--;
		}
	}

}
