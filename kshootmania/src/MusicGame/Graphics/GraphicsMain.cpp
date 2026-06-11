#include "GraphicsMain.hpp"
#include "GraphicsDefines.hpp"
#include "Graphics/ImageUtils.hpp"
#include "MusicGame/GameDefines.hpp"
#include "MusicGame/HispeedUtils.hpp"
#include "kson/Util/GraphUtils.hpp"

namespace MusicGame::Graphics
{
	void ScoreAnimator::update(int32 score)
	{
		if (m_targetScore != score)
		{
			m_startScore = m_displayedScore;
			m_targetScore = score;
			m_animationTimer.restart();
		}

		if (m_animationTimer.isRunning() && m_animationTimer < kAnimationDuration)
		{
			const double progress = m_animationTimer.sF() / kAnimationDuration.count();
			m_displayedScore = static_cast<int32>(m_startScore + static_cast<double>(m_targetScore - m_startScore) * progress);
		}
		else
		{
			m_displayedScore = m_targetScore;
		}
	}

	int32 ScoreAnimator::displayedScore() const
	{
		return m_displayedScore;
	}

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

		constexpr FilePathView kDefaultLayerFilePath = U"imgs/bg/arrow.gif";

		std::array<Texture, 2> LoadBGTextures(const kson::ChartData& chartData, FilePathView parentPath)
		{
			std::array<Texture, 2> textures;

			for (size_t i = 0; i < chartData.bg.legacy.bg.size(); ++i)
			{
				const String filename = Unicode::FromUTF8(chartData.bg.legacy.bg[i].filename);

				if (filename.isEmpty())
				{
					if (i == 0)
					{
						// bg[0]が空の場合はデフォルトの背景
						textures[i] = Texture(U"imgs/bg/desert{}.jpg"_fmt(i));
					}
					else
					{
						// bg[1]が空の場合はbg[0]と同じテクスチャを使用
						textures[i] = textures[0];
					}
					continue;
				}

				if (FileSystem::Extension(filename).empty())
				{
					// 標準の背景
					const String filePath = U"imgs/bg/{}{}.jpg"_fmt(filename, i);
					if (FileSystem::Exists(filePath))
					{
						textures[i] = Texture(filePath);
					}
					else
					{
						// 存在しない場合はデフォルトの背景
						textures[i] = Texture(U"imgs/bg/desert{}.jpg"_fmt(i));
					}
				}
				else
				{
					// 標準の背景でなければ、譜面ファイルと同じディレクトリのファイル名として参照
					const String filePath = FileSystem::PathAppend(parentPath, filename);
					if (FileSystem::Exists(filePath))
					{
						textures[i] = Texture(filePath);
					}
					else
					{
						// 存在しない場合は、デフォルトの背景
						textures[i] = Texture(U"imgs/bg/desert{}.jpg"_fmt(i));
					}
				}
			}

			return textures;
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
				return FilePath{ kDefaultLayerFilePath };
			}
			return filePath;
		}

		FilePath MovieFilePath(const kson::ChartData& chartData, FilePathView parentPath)
		{
			const String filename = Unicode::FromUTF8(chartData.bg.legacy.movie.filename);
			if (filename.isEmpty())
			{
				return U"";
			}

			// 動画は譜面ファイルと同じディレクトリから読み込む
			return FileSystem::PathAppend(parentPath, filename);
		}

		std::array<Array<Texture>, 2> LayerFrameTexturesFromTileGrid(const Grid<Image>& tiles)
		{
			std::array<Array<Texture>, 2> frameTextures;
			const size_t rowCount = Min<size_t>(tiles.height(), frameTextures.size());
			for (size_t row = 0; row < rowCount; ++row)
			{
				frameTextures[row].reserve(tiles.width());
				for (size_t col = 0; col < tiles.width(); ++col)
				{
					frameTextures[row].emplace_back(tiles[row][col]);
				}
			}
			return frameTextures;
		}

		std::array<Array<Texture>, 2> LoadLayerFrameTextures(FilePathView layerFilePath)
		{
			// 通常サイズの画像はImageで直接デコード
			if (const Image image{ layerFilePath })
			{
				return LayerFrameTexturesFromTileGrid(ImageUtils::SplitIntoTiles(image, kLayerFrameTextureSize));
			}

			// Image単体のサイズ制限を超える画像はstb_image経由でタイル単位に分割して読み込む
			const Grid<Image> tiles = ImageUtils::LoadAsTileGrid(layerFilePath, kLayerFrameTextureSize);
			if (!tiles.isEmpty())
			{
				return LayerFrameTexturesFromTileGrid(tiles);
			}

			Logger << U"[ksm warning] Failed to load layer image: " << layerFilePath;

			// 読み込みに失敗した場合はデフォルトのレイヤーアニメーションへフォールバック
			if (layerFilePath != kDefaultLayerFilePath)
			{
				Logger << U"[ksm info] Falling back to default layer: " << kDefaultLayerFilePath;
				return LoadLayerFrameTextures(kDefaultLayerFilePath);
			}
			return {};
		}

		/// @brief BPMを文字列にフォーマット(小数部分がある場合のみ小数を表示)
		String FormatBPM(double bpm)
		{
			const int32 bpmInt = static_cast<int32>(bpm * 1000);
			const int32 integerPart = bpmInt / 1000;
			const int32 decimalPart = bpmInt % 1000;
			if (decimalPart == 0)
			{
				return Format(integerPart);
			}

			// 末尾のゼロを除去
			String result = U"{}.{:03d}"_fmt(integerPart, decimalPart);
			while (result.back() == U'0')
			{
				result.pop_back();
			}
			return result;
		}

		std::shared_ptr<noco::Canvas> LoadHUDCanvas()
		{
			const FilePath uiFilePath = FsUtils::GetResourcePath(U"ui/scene/play_hud.noco");
			const auto canvas = noco::Canvas::LoadFromFile(uiFilePath);
			if (!canvas)
			{
				throw Error{ U"Failed to load '{}'"_fmt(uiFilePath) };
			}
			return canvas;
		}

		constexpr double kPreStartOffsetSec = 3.4;
		constexpr double kPreStartOffsetWithMovieSec = 4.4;
	}

	void GraphicsMain::drawBG(const ViewStatus& viewStatus) const
	{
		const ScopedRenderStates3D samplerState(SamplerState::ClampNearest);

		// ゲージパーセンテージに応じてBGテクスチャのインデックスを決定
		const int32 percentThreshold = (m_playOption.gaugeType == GaugeType::kHardGauge || m_playOption.gameMode == GameMode::kCourseMode) ? kGaugePercentageThresholdHardWarning : kGaugePercentageThreshold;
		const int32 bgTextureIndex = viewStatus.gaugePercentageInt >= percentThreshold ? 1 : 0;

		double bgTiltRadians = viewStatus.tiltRadiansForBgLayer / 3;
		m_bgBillboardMesh.draw(m_bgTransform * TiltTransformMatrix(bgTiltRadians, kBGBillboardPosition), m_bgTextures[bgTextureIndex]);
	}

	void GraphicsMain::drawLayer(const kson::ChartData& chartData, const GameStatus& gameStatus, const ViewStatus& viewStatus) const
	{
		const ScopedRenderStates3D samplerState(SamplerState::ClampNearest);
		const ScopedRenderStates3D renderState(BlendState::Additive);

		double layerTiltRadians = 0.0;
		if (chartData.bg.legacy.layer.rotation.tilt)
		{
			layerTiltRadians += viewStatus.tiltRadiansForBgLayer * 0.8;
		}
		if (chartData.bg.legacy.layer.rotation.spin)
		{
			layerTiltRadians += Math::ToRadians(viewStatus.camStatus.rotationDegLayer);
		}

		// ゲージパーセンテージに応じてレイヤーテクスチャのインデックスを決定
		const int32 percentThreshold = (m_playOption.gaugeType == GaugeType::kHardGauge || m_playOption.gameMode == GameMode::kCourseMode) ? kGaugePercentageThresholdHardWarning : kGaugePercentageThreshold;
		const int32 layerTextureIndex = viewStatus.gaugePercentageInt >= percentThreshold ? 1 : 0;

		if (!m_layerFrameTextures[layerTextureIndex].empty())
		{
			// レイヤーアニメーション速度の計算
			int32 layerFrame = 0;
			const int32 duration = chartData.bg.legacy.layer.duration;
			if (duration == 0)
			{
				// duration == 0の場合、テンポ同期(1フレーム = 0.035小節)
				const kson::Pulse relativePulse = gameStatus.currentPulse - m_layerFrameOriginPulse;
				layerFrame = MathUtils::WrappedMod(static_cast<int32>(relativePulse * 1000 / 35 / kson::kResolution4), static_cast<int32>(m_layerFrameTextures[layerTextureIndex].size()));
			}
			else
			{
				// duration != 0の場合、固定速度(ミリ秒単位)
				const double absDuration = std::abs(duration);
				const double frameTimeMs = (gameStatus.currentTimeSec - m_layerFrameOriginTimeSec) * 1000.0;
				const int32 frameCount = static_cast<int32>(m_layerFrameTextures[layerTextureIndex].size());

				if (duration > 0)
				{
					// 正再生
					layerFrame = MathUtils::WrappedMod(static_cast<int32>(frameTimeMs * frameCount / absDuration), frameCount);
				}
				else
				{
					// 逆再生
					layerFrame = MathUtils::WrappedMod(frameCount - static_cast<int32>(frameTimeMs * frameCount / absDuration) - 1, frameCount);
				}
			}

			m_bgBillboardMesh.draw(m_layerTransform * TiltTransformMatrix(layerTiltRadians, kLayerBillboardPosition), m_layerFrameTextures[layerTextureIndex].at(layerFrame));
		}
	}

	GraphicsMain::GraphicsMain(const kson::ChartData& chartData, const kson::TimingCache& timingCache, FilePathView parentPath, const PlayOption& playOption)
		: m_camera(Scene::Size(), kCameraVerticalFOV, kCameraPosition, kCameraLookAt)
		, m_bgBillboardMesh(MeshData::Billboard())
		, m_bgTextures(LoadBGTextures(chartData, parentPath))
		, m_bgTransform(m_camera.billboard(kBGBillboardPosition, kBGBillboardSize))
		, m_layerFrameTextures(LoadLayerFrameTextures(LayerFilePath(chartData, parentPath)))
		, m_layerTransform(m_camera.billboard(kLayerBillboardPosition, kLayerBillboardSize))
		, m_jdgoverlay3DGraphics(m_camera)
		, m_hudCanvas(LoadHUDCanvas())
		, m_gaugePanel(ToGaugeCalcType(playOption.gaugeType, playOption.gameMode))
		, m_laserApproachIndicator(chartData)
		, m_moviePanel(MovieFilePath(chartData, parentPath), chartData.bg.legacy.movie.offset / 1000.0 / playOption.nonZeroPlaybackSpeed(), playOption.playbackSpeed, playOption.movieEnabled)
		, m_playOption(playOption)
		, m_layerFrameOriginTimeSec(-TimeSecBeforeStart(m_moviePanel.isEnabled()).count())
		, m_layerFrameOriginPulse(static_cast<kson::Pulse>(kson::SecToPulseDouble(m_layerFrameOriginTimeSec, chartData.beat, timingCache)))
	{
		// HUDのCanvasパラメータを初期設定
		const double startBPM = chartData.beat.bpm.contains(0) ? chartData.beat.bpm.at(0) : kDefaultBPM;

		String title = Unicode::FromUTF8(chartData.meta.title);
		String artist = Unicode::FromUTF8(chartData.meta.artist);
		FilePath titleImgPath;
		FilePath artistImgPath;

		if (!chartData.meta.titleImgFilename.empty())
		{
			titleImgPath = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(chartData.meta.titleImgFilename));
			if (FileSystem::IsFile(titleImgPath))
			{
				title = U"";
			}
			else
			{
				titleImgPath.clear();
			}
		}

		if (!chartData.meta.artistImgFilename.empty())
		{
			artistImgPath = FileSystem::PathAppend(parentPath, Unicode::FromUTF8(chartData.meta.artistImgFilename));
			if (FileSystem::IsFile(artistImgPath))
			{
				artist = U"";
			}
			else
			{
				artistImgPath.clear();
			}
		}

		m_hudCanvas->setParamValues({
			{ U"title", title },
			{ U"titleImgFilePath", titleImgPath },
			{ U"artist", artist },
			{ U"artistImgFilePath", artistImgPath },
			{ U"levelNumber", chartData.meta.level },
			{ U"bpmNumberText", FormatBPM(startBPM) },
			{ U"difficultyIndex", chartData.meta.difficulty.idx },
			{ U"hispeedValueText", HispeedUtils::ToDisplayString(playOption.hispeedSetting) },
			{ U"hispeedValueTextEffective", Format(playOption.hispeedSetting.value) },
			{ U"jacketFilePath", FsUtils::ResolveJacketPath(parentPath, Unicode::FromUTF8(chartData.meta.jacketFilename)) },
		});
	}

	void GraphicsMain::prepareMovie(double globalOffsetSec)
	{
		m_moviePanel.prepare(globalOffsetSec);
	}

	void GraphicsMain::update(const GameStatus& gameStatus, const ViewStatus& viewStatus, const kson::TimingCache& timingCache)
	{
		m_comboOverlay.update(viewStatus);
		m_scoreAnimator.update(viewStatus.score);
		m_highway3DGraphics.update(viewStatus);
		m_laserApproachIndicator.update(gameStatus, timingCache);
		m_moviePanel.update(gameStatus.currentTimeSec, gameStatus.isPaused);

		// HUDのCanvasパラメータを更新
		m_hudCanvas->setParamValues({
			{ U"scoreNumber", m_scoreAnimator.displayedScore() },
			{ U"fpsNumber", static_cast<int32>(Profiler::FPS()) },
			{ U"bpmNumberText", FormatBPM(gameStatus.currentBPM) },
			{ U"timingAdjustVisible", viewStatus.timingAdjustMs != 0 },
			{ U"timingAdjustSignIndex", viewStatus.timingAdjustMs > 0 ? 1 : 0 },
			{ U"timingAdjustNumber", Abs(viewStatus.timingAdjustMs) },
		});

		m_hudCanvas->update();
	}

	void GraphicsMain::draw(const kson::ChartData& chartData, const std::array<HashSet<kson::Pulse>, kson::kNumLaserLanesSZ>& laserCurvedPulses, const kson::TimingCache& timingCache, const GameStatus& gameStatus, const ViewStatus& viewStatus, const Scroll::HighwayScrollContext& highwayScrollContext, Duration bgmDuration) const
	{
		// 各レンダーテクスチャを用意
		m_highway3DGraphics.draw2D(chartData, laserCurvedPulses, m_playOption, timingCache, gameStatus, viewStatus, highwayScrollContext);
		m_jdgoverlay3DGraphics.draw2D(gameStatus, viewStatus);
		Graphics2D::Flush();

		// 3D空間を描画
		Graphics3D::SetCameraTransform(m_camera);
		if (m_playOption.showBG)
		{
			drawBG(viewStatus);
		}
		if (m_playOption.showLayer)
		{
			drawLayer(chartData, gameStatus, viewStatus);
		}
		m_highway3DGraphics.draw3D(viewStatus);
		m_jdgline3DGraphics.draw3D(viewStatus);
		m_jdgoverlay3DGraphics.draw3D(viewStatus);
		m_laserCursor3DGraphics.draw3D(gameStatus, viewStatus, m_camera);

		// 手前に表示する2DのHUDを描画
		{
			const double preStartOffset = m_moviePanel.isEnabled() ? kPreStartOffsetWithMovieSec : kPreStartOffsetSec;
			const double totalDurationSec = bgmDuration.count() + preStartOffset;
			const double progress = Clamp(gameStatus.currentTimeSec / totalDurationSec, 0.0, 1.0);

			m_hudCanvas->setParamValues({
				{ U"positionMarkerProgress", progress },
				{ U"hispeedValueText", HispeedUtils::ToDisplayString(highwayScrollContext.highwayScroll().hispeedSetting()) },
				{ U"hispeedValueTextEffective", Format(highwayScrollContext.currentHispeedWithScrollSpeed()) },
			});
		}

		m_hudCanvas->draw();
		m_gaugePanel.draw(viewStatus.gaugePercentage, gameStatus.currentPulse);
		m_comboOverlay.draw(gameStatus.currentTimeSec);
		m_achievementPanel.draw(gameStatus);
		m_laserApproachIndicator.draw(gameStatus.currentTimeSec);
		m_moviePanel.draw();

		// HARDゲージ落ち時の赤色オーバーレイ
		if (gameStatus.playFinishStatus.has_value() && gameStatus.playFinishStatus->isHardFailed)
		{
			constexpr double kFadeTimeSec = 0.75;
			const double elapsedSec = gameStatus.currentTimeSec - gameStatus.playFinishStatus->finishTimeSec;
			const double t = Clamp(elapsedSec / kFadeTimeSec, 0.0, 1.0);
			const uint8 g = static_cast<uint8>(48 + 128 - static_cast<int32>(128 * t));
			const Color overlayColor{ 255, g, 0 };

			const ScopedRenderStates2D blend{ BlendState::Additive };
			Scene::Rect().draw(overlayColor);
		}
	}

	bool GraphicsMain::hasMovie() const
	{
		return m_moviePanel.isEnabled();
	}

	void GraphicsMain::seekMoviePosSec(SecondsF posSec)
	{
		m_moviePanel.seekPosSec(posSec);
	}
}
