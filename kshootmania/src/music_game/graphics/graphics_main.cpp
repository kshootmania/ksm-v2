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

		FilePath BGFilePath(const kson::ChartData& chartData)
		{
			const String bgFilename = Unicode::FromUTF8(chartData.bg.legacy.bg.at(0).filename);
			if (FileSystem::Extension(bgFilename).empty())
			{
				// 標準の背景
				return U"imgs/bg/{}0.jpg"_fmt(bgFilename);
			}

			// TODO: Convert to absolute path using chart file path parent directory
			return bgFilename;
		}

		FilePath LayerFilePath(const kson::ChartData& chartData)
		{
			const String layerFilename = Unicode::FromUTF8(chartData.bg.legacy.layer.filename);
			if (FileSystem::Extension(layerFilename).empty())
			{
				// 標準のレイヤーアニメーション
				return U"imgs/bg/{}.gif"_fmt(layerFilename);
			}

			// TODO: Convert to absolute path using chart file path parent directory
			return layerFilename;
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

	void GraphicsMain::drawBG() const
	{
		const ScopedRenderStates3D samplerState(SamplerState::ClampNearest);
		double bgTiltRadians = m_highwayTilt.radians() / 3;
		m_bgBillboardMesh.draw(m_bgTransform * TiltTransformMatrix(bgTiltRadians, kBGBillboardPosition), m_bgTexture);
	}

	void GraphicsMain::drawLayer(const kson::ChartData& chartData, const GameStatus& gameStatus) const
	{
		const ScopedRenderStates3D samplerState(SamplerState::ClampNearest);
		const ScopedRenderStates3D renderState(BlendState::Additive);

		double layerTiltRadians = 0.0;
		if (chartData.bg.legacy.layer.rotation.tilt)
		{
			layerTiltRadians += m_highwayTilt.radians() * 0.8;
		}

		// TODO: Layer speed specified by KSH
		// TODO: Use different layer texture index depending on gauge percentage
		if (!m_layerFrameTextures[0].empty())
		{
			const int32 layerFrame = MathUtils::WrappedMod(static_cast<int32>(gameStatus.currentPulse * 1000 / 35 / kson::kResolution4), static_cast<int32>(m_layerFrameTextures[0].size()));
			m_bgBillboardMesh.draw(m_layerTransform * TiltTransformMatrix(layerTiltRadians, kLayerBillboardPosition), m_layerFrameTextures[0].at(layerFrame));
		}
	}

	GraphicsMain::GraphicsMain(const kson::ChartData& chartData, FilePathView parentPath)
		: m_camera(Scene::Size(), kCameraVerticalFOV, kCameraPosition, kCameraLookAt)
		, m_bgBillboardMesh(MeshData::Billboard())
		, m_bgTexture(BGFilePath(chartData))
		, m_bgTransform(m_camera.billboard(kBGBillboardPosition, kBGBillboardSize))
		, m_layerFrameTextures(SplitLayerTexture(LayerFilePath(chartData)))
		, m_layerTransform(m_camera.billboard(kLayerBillboardPosition, kLayerBillboardSize))
		, m_jdgoverlay3DGraphics(m_camera)
		, m_songInfoPanel(chartData, parentPath)
		, m_gaugePanel(kNormalGauge/* TODO: gauge type */)
	{
	}

	void GraphicsMain::update(const kson::ChartData& chartData, const GameStatus& gameStatus)
	{
		const double leftLaserValue = kson::GraphSectionValueAtWithDefault(chartData.note.laser[0], gameStatus.currentPulse, 0.0); // range: [0, +1]
		const double rightLaserValue = kson::GraphSectionValueAtWithDefault(chartData.note.laser[1], gameStatus.currentPulse, 1.0) - 1.0; // range: [-1, 0]
		const double tiltFactor = leftLaserValue + rightLaserValue; // range: [-1, +1]
		m_highwayTilt.update(tiltFactor);
	}

	void GraphicsMain::draw(const kson::ChartData& chartData, const GameStatus& gameStatus) const
	{
		// 各レンダーテクスチャを用意
		m_highway3DGraphics.draw2D(chartData, gameStatus);
		m_jdgoverlay3DGraphics.draw2D(gameStatus);
		Graphics2D::Flush();

		// 3D空間を描画
		Graphics3D::SetCameraTransform(m_camera);
		drawBG();
		drawLayer(chartData, gameStatus);
		const double tiltRadians = m_highwayTilt.radians();
		m_highway3DGraphics.draw3D(tiltRadians);
		m_jdgline3DGraphics.draw3D(tiltRadians);
		m_jdgoverlay3DGraphics.draw3D(tiltRadians);
		m_laserCursor3DGraphics.draw3D(tiltRadians, gameStatus);

		// 手前に表示する2DのHUDを描画
		m_songInfoPanel.draw(gameStatus.currentBPM);
		m_scorePanel.draw(gameStatus.score);
		m_gaugePanel.draw(100.0/* TODO: Percentage */, gameStatus.currentPulse);
		m_frameRateMonitor.draw();
	}
}
