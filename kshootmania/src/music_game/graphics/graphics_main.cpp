#include "graphics_main.hpp"
#include "graphics_defines.hpp"
#include "music_game/game_defines.hpp"
#include "kson/util/graph_utils.hpp"

namespace MusicGame::Graphics
{
	namespace
	{
		constexpr double kCameraVerticalFOV = 45_deg;
		constexpr Vec3 kCameraPosition = { 0.0, 45.0, -366.0 };

		constexpr double kSin15Deg = 0.2588190451;
		constexpr double kCos15Deg = 0.9659258263;
		constexpr Vec3 kCameraLookAt = kCameraPosition + Vec3{ 0.0, -100.0 * kSin15Deg, 100.0 * kCos15Deg };

		constexpr Float3 kBGBillboardPosition = Float3{ 0, -52.5f, 0 };
		constexpr Float2 kBGBillboardSize = Float2{ 900.0f, 800.0f } *0.65f;

		constexpr Size kLayerFrameTextureSize = { 600, 480 };
		constexpr Float3 kLayerBillboardPosition = Float3{ 0, -41.0f, 0 };
		constexpr Float2 kLayerBillboardSize = Float2{ 880.0f, 704.0f } * 0.65f;

		FilePath BGFilePath(const kson::ChartData& chartData, FilePathView parentPath)
		{
			const String filename = Unicode::FromUTF8(chartData.bg.legacy.bg[0].filename);
			if (FileSystem::Extension(filename).empty())
			{
				// 標準の背景
				return U"imgs/bg/{}0.jpg"_fmt(filename);
			}

			// 標準の背景でなければ、譜面ファイルと同じディレクトリのファイル名として参照
			const String filePath = FileSystem::PathAppend(parentPath, filename);
			if (!FileSystem::Exists(filePath))
			{
				// 存在しない場合は、デフォルトの背景(desert)を返す
				return U"imgs/bg/desert0.jpg";
			}
			return filePath;
		}

		FilePath LayerFilePath(const kson::ChartData& chartData, FilePathView parentPath)
		{
			const String filename = Unicode::FromUTF8(chartData.bg.legacy.layer.filename);
			if (FileSystem::Extension(filename).empty())
			{
				// 標準のレイヤーアニメーション
				return U"imgs/bg/{}.gif"_fmt(filename);
			}

			// 標準のレイヤーアニメーションでなければ、譜面ファイルと同じディレクトリのファイル名として参照
			const String filePath = FileSystem::PathAppend(parentPath, filename);
			if (!FileSystem::Exists(filePath))
			{
				// 存在しない場合は、デフォルトのレイヤーアニメーション(arrow)を返す
				return U"imgs/bg/arrow.gif";
			}
			return filePath;
		}

		std::array<Array<RenderTexture>, 2> SplitLayerTexture(FilePathView layerFilePath)
		{
			const TiledTexture tiledTexture(Texture(layerFilePath),
				{
					.row = TiledTextureSizeInfo::kAutoDetect,
					.column = TiledTextureSizeInfo::kAutoDetect,
					.sourceSize = kLayerFrameTextureSize,
				});

			std::array<Array<RenderTexture>, 2> renderTextures;
			for (int32 i = 0; i < 2; ++i)
			{
				renderTextures[i].reserve(tiledTexture.column());
				for (int32 j = 0; j < tiledTexture.column(); ++j)
				{
					const RenderTexture renderTexture(kLayerFrameTextureSize, Palette::Black);
					const ScopedRenderTarget2D renderTarget(renderTexture);
					Shader::Copy(tiledTexture(i, j), renderTexture);
					renderTextures[i].push_back(std::move(renderTexture));
				}
			}

			return renderTextures;
		}
	}

	void GraphicsMain::drawBG(const ViewStatus& viewStatus) const
	{
		const ScopedRenderStates3D samplerState(SamplerState::ClampNearest);
		double bgTiltRadians = viewStatus.tiltRadians / 3;
		m_bgBillboardMesh.draw(m_bgTransform * TiltTransformMatrix(bgTiltRadians, kBGBillboardPosition), m_bgTexture);
	}

	void GraphicsMain::drawLayer(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus) const
	{
		const ScopedRenderStates3D samplerState(SamplerState::ClampNearest);
		const ScopedRenderStates3D renderState(BlendState::Additive);

		double layerTiltRadians = 0.0;
		if (chartData.bg.legacy.layer.rotation.tilt)
		{
			layerTiltRadians += viewStatus.tiltRadians * 0.8 + Math::ToRadians(viewStatus.camStatus.rotationZLayer);
		}

		// TODO: Layer speed specified by KSH
		// TODO: Use different layer texture index depending on gauge percentage
		if (!m_layerFrameTextures[0].empty())
		{
			const int32 layerFrame = MathUtils::WrappedMod(static_cast<int32>(gameStatus.currentPulse * 1000 / 35 / kson::kResolution4), static_cast<int32>(m_layerFrameTextures[0].size()));
			m_bgBillboardMesh.draw(m_layerTransform * TiltTransformMatrix(layerTiltRadians, kLayerBillboardPosition), m_layerFrameTextures[0].at(layerFrame));
		}
	}

	GraphicsMain::GraphicsMain(const kson::ChartData& chartData, FilePathView parentPath, GaugeType gaugeType)
		: m_camera(Scene::Size(), kCameraVerticalFOV, kCameraPosition, kCameraLookAt)
		, m_bgBillboardMesh(MeshData::Billboard())
		, m_bgTexture(BGFilePath(chartData, parentPath))
		, m_bgTransform(m_camera.billboard(kBGBillboardPosition, kBGBillboardSize))
		, m_layerFrameTextures(SplitLayerTexture(LayerFilePath(chartData, parentPath)))
		, m_layerTransform(m_camera.billboard(kLayerBillboardPosition, kLayerBillboardSize))
		, m_jdgoverlay3DGraphics(m_camera)
		, m_songInfoPanel(chartData, parentPath)
		, m_gaugePanel(gaugeType)
	{
	}

	void GraphicsMain::update(const ViewStatus& viewStatus)
	{
		m_comboOverlay.update(viewStatus);
		m_highway3DGraphics.update(viewStatus);
	}

	void GraphicsMain::draw(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext) const
	{
		// 各レンダーテクスチャを用意
		m_highway3DGraphics.draw2D(chartData, gameStatus, viewStatus, highwayScrollContext);
		m_jdgoverlay3DGraphics.draw2D(gameStatus, viewStatus);
		Graphics2D::Flush();

		// 3D空間を描画
		Graphics3D::SetCameraTransform(m_camera);
		drawBG(viewStatus);
		drawLayer(chartData, gameStatus, viewStatus);
		m_highway3DGraphics.draw3D(gameStatus, viewStatus);
		m_jdgline3DGraphics.draw3D(gameStatus, viewStatus);
		m_jdgoverlay3DGraphics.draw3D(gameStatus, viewStatus);
		m_laserCursor3DGraphics.draw3D(gameStatus, viewStatus, m_camera);

		// 手前に表示する2DのHUDを描画
		m_songInfoPanel.draw(gameStatus.currentBPM, highwayScrollContext);
		m_scorePanel.draw(viewStatus.score);
		m_gaugePanel.draw(viewStatus.gaugePercentage, gameStatus.currentPulse);
		m_comboOverlay.draw();
		m_frameRateMonitor.draw();
	}
}
