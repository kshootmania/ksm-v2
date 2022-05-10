#include "kson/editor/editor_info.hpp"

bool kson::EditorInfo::empty() const
{
	return comment.empty();
}

void kson::to_json(nlohmann::json& j, const EditorInfo& editor)
{
	j = nlohmann::json::object();

	if (!editor.comment.empty())
	{
		j["comment"] = editor.comment;
	}
}
