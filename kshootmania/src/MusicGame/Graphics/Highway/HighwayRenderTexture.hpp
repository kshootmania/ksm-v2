#pragma once

namespace MusicGame::Graphics
{
	/// @brief highway_wipeによるレーンのテクスチャ上の描画範囲
	struct HighwayWipeRegion
	{
		// 通常のレーンの範囲
		int32 baseY = 0;
		int32 baseHeight = 0;

		// レーン消失の範囲
		int32 wipeY = 0;
		int32 wipeHeight = 0;
	};

	/// @brief highway_wipeの値から描画範囲を取得
	HighwayWipeRegion CalcHighwayWipeRegion(double highwayWipe);

	/// @brief テクスチャ領域のうち、レーン消失の範囲に入っている部分のみ描画
	/// @param region 描画するテクスチャ領域
	/// @param position 描画位置(左上)
	/// @param wipeRegion レーン消失の範囲
	/// @param color 乗算する色
	void DrawInHighwayWipeRegion(const TextureRegion& region, const Vec2& position, const HighwayWipeRegion& wipeRegion, const ColorF& color = Palette::White);

	struct HighwayRenderTexture
	{
	private:
		const RenderTexture m_additiveTexture;
		const RenderTexture m_invMultiplyTexture;
		const RenderTexture m_additiveBaseTexture;
		const RenderTexture m_invMultiplyBaseTexture;
		const RenderTexture m_additiveBaseWipeTexture;
		const RenderTexture m_invMultiplyBaseWipeTexture;

	public:
		HighwayRenderTexture();

		void draw3D(const DynamicMesh& mesh) const;

		const RenderTexture& additiveTexture() const;

		const RenderTexture& invMultiplyTexture() const;

		void drawBaseTexture(double centerSplit, double highwayWipe) const;
	};
}
