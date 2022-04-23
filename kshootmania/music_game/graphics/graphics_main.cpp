#include "graphics_main.hpp"
#include "graphics_defines.hpp"
#include "music_game/game_defines.hpp"
#include "ksh/util/graph_utils.hpp"

namespace
{
	constexpr double kCameraVerticalFOV = 45_deg;
	constexpr Vec3 kCameraPosition = { 0.0, 45.0, -366.0 };

	constexpr double kSin15Deg = 0.2588190451;
	constexpr double kCos15Deg = 0.9659258263;
	constexpr Vec3 kCameraLookAt = kCameraPosition + Vec3{ 0.0, -100.0 * kSin15Deg, 100.0 * kCos15Deg };

	constexpr Float3 kBGBillboardPosition = Float3{ 0, -53.0f, 0 };
	constexpr Float2 kBGBillboardSize = Float2{ 900.0f, 800.0f } * 0.655f;

	constexpr Size kLayerFrameSize = { 600, 480 };
	constexpr Float3 kLayerBillboardPosition = Float3{ 0, -31.0f, 0 };
	constexpr Float2 kLayerBillboardSize = Float2{ 880.0f, 704.0f } * 0.655f;

	FilePath BGFilePath(const ksh::ChartData& chartData)
	{
		const String bgFilename = Unicode::Widen(ksh::UnU8(chartData.bg.legacy.bgInfos.at(0).filename));
		if (FileSystem::Extension(bgFilename).empty())
		{
			// Built-in BG textures
			return U"imgs/bg/{}0.jpg"_fmt(bgFilename);
		}

		// TODO: Convert to absolute path using chart file path parent directory
		return bgFilename;
	}

	FilePath LayerFilePath(const ksh::ChartData& chartData)
	{
		const String layerFilename = Unicode::Widen(ksh::UnU8(chartData.bg.legacy.layerInfos.at(0).filename));
		if (FileSystem::Extension(layerFilename).empty())
		{
			// Built-in BG textures
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
				.sourceSize = kLayerFrameSize,
			});

		std::array<Array<RenderTexture>, 2> renderTextures;
		for (int32 i = 0; i < 2; ++i)
		{
			renderTextures[i].reserve(tiledTexture.column());
			for (int32 j = 0; j < tiledTexture.column(); ++j)
			{
				const RenderTexture renderTexture(kLayerFrameSize, Palette::Black);
				const ScopedRenderTarget2D renderTarget(renderTexture);
				tiledTexture(i, j).draw();
				renderTextures[i].push_back(std::move(renderTexture));
			}
		}

		return renderTextures;
	}

	Mat4x4 TiltMatrix(double radians, const Float3& center)
	{
		return Mat4x4::Rotate(Float3::Backward(), radians, center);
	}
}

MusicGame::Graphics::GraphicsMain::GraphicsMain(const ksh::ChartData& chartData, const ksh::TimingCache& timingCache)
	: m_camera(Scene::Size(), kCameraVerticalFOV, kCameraPosition, kCameraLookAt)
	, m_billboardMesh(MeshData::Billboard())
	, m_3dViewTexture(Scene::Size(), TextureFormat::R8G8B8A8_Unorm, HasDepth::No)
	, m_bgTexture(BGFilePath(chartData))
	, m_bgTransform(m_camera.billboard(kBGBillboardPosition, kBGBillboardSize))
	, m_layerFrameTextures(SplitLayerTexture(LayerFilePath(chartData)))
	, m_layerTransform(m_camera.billboard(kLayerBillboardPosition, kLayerBillboardSize))
	, m_songInfoPanel(chartData)
	, m_gaugePanel(kNormalGauge/* TODO: gauge type */, chartData.beat.resolution)
	, m_initialPulse(ksh::TimingUtils::MsToPulse(TimeSecBeforeStart(false/* TODO: movie */), chartData.beat, timingCache))
{
}

void MusicGame::Graphics::GraphicsMain::update(const UpdateInfo& updateInfo)
{
	m_updateInfo = updateInfo;

	double tiltFactor = 0.0;
	if (updateInfo.pChartData != nullptr)
	{
		const ksh::ChartData& chartData = *updateInfo.pChartData;
		const double leftLaserValue = ksh::GraphSectionValueAtWithDefault(chartData.note.laserLanes[0], updateInfo.currentPulse, 0.0); // range: [0, +1]
		const double rightLaserValue = ksh::GraphSectionValueAtWithDefault(chartData.note.laserLanes[1], updateInfo.currentPulse, 1.0) - 1.0; // range: [-1, 0]
		tiltFactor = leftLaserValue + rightLaserValue; // range: [-1, +1]
	}
	m_highwayTilt.update(tiltFactor);
}

void MusicGame::Graphics::GraphicsMain::draw() const
{
	assert(m_updateInfo.pChartData != nullptr);

	Graphics3D::SetCameraTransform(m_camera);

	const double tiltRadians = m_highwayTilt.radians();

	// Draw BG texture
	{
		const ScopedRenderStates3D samplerState(SamplerState::ClampNearest);
		double bgTiltRadians = 0.0;
		if (m_updateInfo.pChartData->bg.legacy.bgInfos[0].rotationFlags.tiltAffected)
		{
			bgTiltRadians += tiltRadians / 3;
		}

		m_billboardMesh.draw(m_bgTransform * TiltMatrix(bgTiltRadians, kBGBillboardPosition), m_bgTexture);
	}

	// Draw layer animation
	{
		const ScopedRenderStates3D samplerState(SamplerState::ClampNearest);
		const ScopedRenderStates3D renderState(BlendState::Additive);

		double layerTiltRadians = 0.0;
		if (m_updateInfo.pChartData->bg.legacy.layerInfos[0].rotationFlags.tiltAffected)
		{
			layerTiltRadians += tiltRadians * 0.8;
		}

		// TODO: Layer speed specified by KSH
		const ksh::Pulse resolution = m_updateInfo.pChartData->beat.resolution;
		const int32 layerFrame = MathUtils::WrappedMod(static_cast<int32>(m_updateInfo.currentPulse * 1000 / 35 / (resolution * 4)), static_cast<int32>(m_layerFrameTextures[0].size()));
		m_billboardMesh.draw(m_layerTransform * TiltMatrix(layerTiltRadians, kLayerBillboardPosition), m_layerFrameTextures[0].at(layerFrame));
	}

	m_highway3DGraphics.draw(m_updateInfo, tiltRadians);

	m_jdgline3DGraphics.draw(m_updateInfo, tiltRadians);

	// Draw 3D scene to 2D scene
	Graphics3D::Flush();
	m_3dViewTexture.resolve();
	m_3dViewTexture.draw();

	m_songInfoPanel.draw();
	m_scorePanel.draw(0/* TODO: Score */);
	m_gaugePanel.draw(100.0/* TODO: Percentage */, m_updateInfo.currentPulse);
	m_frameRateMonitor.draw();
}
