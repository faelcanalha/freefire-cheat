#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_internal.h"


namespace Custom {
	bool AnimButton(const char* label, const ImVec2& size_arg);
	bool ButtonEx_animation(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags);

	bool BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags flags);
	bool BeginChild(const char* str_id, const ImVec2& size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0);
	bool BeginChild(ImGuiID id, const ImVec2& size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0);
	void EndChild();

	bool Configuration(const char* label, const ImVec2& size_arg);
	bool ButtonEx_Configuration(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags);

	bool Tab(bool selected, const char* icon_label, const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = 0);
	bool Checkbox(const char* label, bool* v);
};


