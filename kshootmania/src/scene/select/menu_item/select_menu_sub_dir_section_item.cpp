#include "select_menu_sub_dir_section_item.hpp"
#include "scene/select/select_menu_graphics.hpp"

SelectMenuSubDirSectionItem::SelectMenuSubDirSectionItem(FilePathView fullPath)
	: m_fullPath(fullPath)
	, m_displayName(FileSystem::FileName(m_fullPath))
{
}

void SelectMenuSubDirSectionItem::decide(const SelectMenuEventContext& context, [[maybe_unused]] int32 difficultyIdx)
{
	//Print << U"Not Implemented (SelectMenuSubDirSectionItem::decide)";
}

void SelectMenuSubDirSectionItem::drawCenter([[maybe_unused]] int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets) const
{
	Shader::Copy(assets.subDirItemTextures.center, renderTexture);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	assets.font(m_displayName).drawAt(44, Vec2{ 16 + 740 / 2, 135 + 102 / 2 });
}

void SelectMenuSubDirSectionItem::drawUpperLower([[maybe_unused]] int32 difficultyIdx, const RenderTexture& renderTexture, const SelectMenuItemGraphicAssets& assets, bool isUpper) const
{
	Shader::Copy(isUpper ? assets.subDirItemTextures.upperHalf : assets.subDirItemTextures.lowerHalf, renderTexture);

	const ScopedRenderTarget2D scopedRenderTarget(renderTexture);
	assets.font(m_displayName).drawAt(38, isUpper ? Vec2{ 16 + 770 / 2, 12 + 86 / 2 } : Vec2{ 16 + 770 / 2, 126 + 86 / 2 });
}
