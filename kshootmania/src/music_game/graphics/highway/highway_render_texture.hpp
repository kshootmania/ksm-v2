#pragma once

namespace MusicGame::Graphics
{
	struct HighwayRenderTexture
	{
	private:
		const RenderTexture m_additiveTexture;
		const RenderTexture m_invMultiplyTexture;
		const RenderTexture m_additiveBaseTexture;
		const RenderTexture m_invMultiplyBaseTexture;

	public:
		HighwayRenderTexture();

		void draw3D(const DynamicMesh& mesh) const;

		const RenderTexture& additiveTexture() const;

		const RenderTexture& invMultiplyTexture() const;

		void drawBaseTexture(double centerSplit) const;
	};
}
