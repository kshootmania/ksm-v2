#pragma once

namespace MusicGame::Graphics
{
	struct HighwayRenderTexture
	{
	private:
		const Point m_offsetPosition;
		const RenderTexture m_additiveTexture;
		const RenderTexture m_invMultiplyTexture;
		const RenderTexture m_additiveBaseTexture;
		const RenderTexture m_invMultiplyBaseTexture;

	public:
		explicit HighwayRenderTexture(bool wide);

		void draw3D(const DynamicMesh& mesh) const;

		const RenderTexture& additiveTexture() const;

		const RenderTexture& invMultiplyTexture() const;

		const Point& offsetPosition() const;

		void clearByBaseTexture() const;
	};
}
