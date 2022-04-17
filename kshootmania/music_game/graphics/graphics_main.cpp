#include "graphics_main.hpp"
#include "music_game/game_defines.hpp"

namespace
{
	constexpr double kCameraVerticalFOV = 45_deg;
	constexpr Vec3 kCameraPosition = { 0.0, 45.0, -366.0 };

	constexpr double kSin15Deg = 0.2588190451;
	constexpr double kCos15Deg = 0.9659258263;
	constexpr Vec3 kCameraLookAt = kCameraPosition + Vec3{ 0.0, -100.0 * kSin15Deg, 100.0 * kCos15Deg };

	constexpr Vec2 kLayerPosition = { 0.0, -35.0 };

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
}

MusicGame::Graphics::GraphicsMain::GraphicsMain(const ksh::ChartData& chartData, const ksh::TimingCache& timingCache)
	: m_camera(Scene::Size(), kCameraVerticalFOV, kCameraPosition, kCameraLookAt)
	, m_additive3dViewTexture(Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes)
	, m_invMultiply3dViewTexture(Scene::Size(), TextureFormat::R8G8B8A8_Unorm_SRGB, HasDepth::Yes)
	, m_bgTexture(BGFilePath(chartData))
	, m_layerTexture(Texture(LayerFilePath(chartData)),
		{
			.row = TiledTextureSizeInfo::kAutoDetect,
			.column = TiledTextureSizeInfo::kAutoDetect,
			.sourceSize = { 600, 480 },
		})
	, m_initialPulse(ksh::TimingUtils::MsToPulse(TimeSecBeforeStart(false/* TODO: movie */), chartData.beat, timingCache))
{
}

void MusicGame::Graphics::GraphicsMain::update(const UpdateInfo& updateInfo)
{
	m_updateInfo = updateInfo;

	m_highway3DGraphics.update(m_updateInfo);
}

void MusicGame::Graphics::GraphicsMain::draw() const
{
	assert(m_updateInfo.pChartData != nullptr);

	Graphics3D::SetCameraTransform(m_camera);

	m_additive3dViewTexture.clear(Palette::Black);
	m_invMultiply3dViewTexture.clear(Palette::Black);

	{
		const ScopedRenderStates2D samplerState(SamplerState::ClampNearest);
		m_bgTexture.resized(ScreenUtils::Scaled(900, 800)).drawAt(Scene::Center());
		{
			const ScopedRenderStates2D renderState(BlendState::Additive);

			// TODO: Layer speed specified by KSH
			const ksh::Pulse resolution = m_updateInfo.pChartData->beat.resolution;
			const int32 layerFrame = MathUtils::WrappedMod(static_cast<int32>(m_updateInfo.currentPulse * 1000 / 35 / (resolution * 4)),  m_layerTexture.column());
			m_layerTexture(0, layerFrame).resized(ScreenUtils::Scaled(880, 704)).drawAt(Scene::Center() + ScreenUtils::Scaled(kLayerPosition));
		}
	}

	m_highway3DGraphics.draw(m_updateInfo, m_additive3dViewTexture, m_invMultiply3dViewTexture);

	// Draw 3D scene to 2D scene
	Graphics3D::Flush();
	m_invMultiply3dViewTexture.resolve();
	{
		const ScopedRenderStates2D renderState(BlendState{ true, Blend::Zero, Blend::InvSrcColor, BlendOp::Add, Blend::Zero, Blend::One, BlendOp::Add });
		m_invMultiply3dViewTexture.draw();
	}
	m_additive3dViewTexture.resolve();
	{
		const ScopedRenderStates2D renderState(BlendState::Additive);
		m_additive3dViewTexture.draw();
	}
}
