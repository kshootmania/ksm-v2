#include "result_panel.hpp"
#include "graphics/number_texture_font.hpp"

namespace
{
	constexpr Size kPanelSize = Size{ 556, 700 } * 2;

	constexpr Size kTopSize = Size{ 600, 50 } * 2;

	int32 GradeToTextureRow(Grade grade)
	{
		assert(grade != Grade::kNoGrade && "Unexpected grade in play result: Grade::kNoGrade");

		return static_cast<int32>(grade) - 1;
	}

	RenderTexture CreateRenderTexture(const kson::ChartData& chartData, const MusicGame::PlayResult& playResult)
	{
		const Texture panelTextureAsset = TextureAsset(ResultTexture::kPanel);
		const NumberTextureFont scoreFont(ResultTexture::kScoreNumberFont, { 64, 64 });
		const NumberTextureFont maxComboFont(ResultTexture::kMaxComboFont, { 136, 120 });
		const NumberTextureFont judgmentCountFont(ResultTexture::kJudgmentCountFont, { 136, 120 });

		RenderTexture renderTexture(panelTextureAsset.size());
		Shader::Copy(panelTextureAsset, renderTexture);

		const ScopedRenderTarget2D renderTarget(renderTexture);
		const ScopedRenderStates2D renderState(SamplerState::ClampAniso);

		// スコアを表示
		const TextureFontTextLayout scoreTextLayout({ 64, 64 }, TextureFontTextLayout::Align::Left, 8, 50.0);
		scoreFont.draw(scoreTextLayout, { 100, 400 }, playResult.score, ZeroPaddingYN::Yes);

		// 最大コンボ数を表示
		const TextureFontTextLayout maxComboTextLayout({ 20, 20 }, TextureFontTextLayout::Align::Right, 4, 27.0);
		maxComboFont.draw(maxComboTextLayout, { 400, 472 }, playResult.maxCombo, ZeroPaddingYN::Yes);

		// 判定内訳の数字を表示
		const TextureFontTextLayout judgmentCountTextLayout({ 26, 24 }, TextureFontTextLayout::Align::Right, 4, 30.0);
		judgmentCountFont.draw(judgmentCountTextLayout, { 354, 532 + 54 * 0 }, playResult.comboStats.critical, ZeroPaddingYN::Yes);
		judgmentCountFont.draw(judgmentCountTextLayout, { 354, 532 + 54 * 1 }, playResult.comboStats.near(), ZeroPaddingYN::Yes);
		judgmentCountFont.draw(judgmentCountTextLayout, { 354, 532 + 54 * 2 }, playResult.comboStats.error, ZeroPaddingYN::Yes);

		// グレードを表示
		const TiledTexture gradeTiledTexture(ResultTexture::kGrade, TiledTextureSizeInfo{
			.row = 6,
			.sourceSize = { 100, 100 },
		});
		gradeTiledTexture(GradeToTextureRow(playResult.grade())).draw(Point{ 430, 286 });

		return renderTexture;
	}

	enum TopTextureRow : int32
	{
		kTopTextureRowFailed = 0,
		kTopTextureRowCleared,
		kTopTextureRowFullCombo,
		kTopTextureRowPerfect,
		kTopTextureRowAssistFailed,
		kTopTextureRowAssistClear,
		kTopTextureRowAssistFullCombo,
		kTopTextureRowAssistPerfect,
		kTopTextureRowEasyFailed,
		kTopTextureRowEasyClear,
		kTopTextureRowEasyFullCombo,
		kTopTextureRowEasyPerfect,
		kTopTextureRowAssistEasyFailed,
		kTopTextureRowAssistEasyClear,
		kTopTextureRowAssistEasyFullCombo,
		kTopTextureRowAssistEasyPerfect,
		kTopTextureRowHardFailed,
		kTopTextureRowHardClear,
		kTopTextureRowAssistHardFailed,
		kTopTextureRowAssistHardClear,
	};

	int32 TopTextureRow(const MusicGame::PlayResult& playResult)
	{
		const Achievement achievement = playResult.achievement();
		const bool isEasyGauge = playResult.gaugeType == MusicGame::GaugeType::kEasyGauge;
		const bool isHardGauge = playResult.gaugeType == MusicGame::GaugeType::kHardGauge;
		const bool isAssist = false; // TODO: アシストかどうかを代入
		switch (achievement)
		{
		case Achievement::kFailed:
			if (isAssist)
			{
				switch (playResult.gaugeType)
				{
				case MusicGame::GaugeType::kNormalGauge:
					return kTopTextureRowAssistFailed;

				case MusicGame::GaugeType::kEasyGauge:
					return kTopTextureRowAssistEasyFailed;

				case MusicGame::GaugeType::kHardGauge:
					return kTopTextureRowAssistHardFailed;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			else
			{
				switch (playResult.gaugeType)
				{
				case MusicGame::GaugeType::kNormalGauge:
					return kTopTextureRowFailed;

				case MusicGame::GaugeType::kEasyGauge:
					return kTopTextureRowEasyFailed;

				case MusicGame::GaugeType::kHardGauge:
					return kTopTextureRowHardFailed;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			break;

		case Achievement::kCleared:
			if (isAssist)
			{
				switch (playResult.gaugeType)
				{
				case MusicGame::GaugeType::kNormalGauge:
					return kTopTextureRowAssistClear;

				case MusicGame::GaugeType::kEasyGauge:
					return kTopTextureRowAssistEasyClear;

				case MusicGame::GaugeType::kHardGauge:
					return kTopTextureRowAssistHardClear;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			else
			{
				switch (playResult.gaugeType)
				{
				case MusicGame::GaugeType::kNormalGauge:
					return kTopTextureRowCleared;

				case MusicGame::GaugeType::kEasyGauge:
					return kTopTextureRowEasyClear;

				case MusicGame::GaugeType::kHardGauge:
					return kTopTextureRowHardClear;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			break;

		case Achievement::kFullCombo:
			if (isAssist)
			{
				switch (playResult.gaugeType)
				{
				case MusicGame::GaugeType::kNormalGauge:
				case MusicGame::GaugeType::kHardGauge:
					return kTopTextureRowAssistFullCombo;

				case MusicGame::GaugeType::kEasyGauge:
					return kTopTextureRowAssistEasyFullCombo;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			else
			{
				switch (playResult.gaugeType)
				{
				case MusicGame::GaugeType::kNormalGauge:
				case MusicGame::GaugeType::kHardGauge:
					return kTopTextureRowFullCombo;

				case MusicGame::GaugeType::kEasyGauge:
					return kTopTextureRowEasyFullCombo;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			break;

		case Achievement::kPerfect:
			if (isAssist)
			{
				switch (playResult.gaugeType)
				{
				case MusicGame::GaugeType::kNormalGauge:
				case MusicGame::GaugeType::kHardGauge:
					return kTopTextureRowAssistPerfect;

				case MusicGame::GaugeType::kEasyGauge:
					return kTopTextureRowAssistEasyPerfect;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			else
			{
				switch (playResult.gaugeType)
				{
				case MusicGame::GaugeType::kNormalGauge:
				case MusicGame::GaugeType::kHardGauge:
					return kTopTextureRowPerfect;

				case MusicGame::GaugeType::kEasyGauge:
					return kTopTextureRowEasyPerfect;

				default:
					assert(false && "Unexpected gauge type in play result");
				}
			}
			break;

		default:
			assert(false && "Unexpected achievement in play result");
		}

		assert(false && "This line should never be reached");
		return kTopTextureRowFailed;
	}
}

ResultPanel::ResultPanel(const kson::ChartData& chartData, const MusicGame::PlayResult& playResult)
	: m_renderTexture(CreateRenderTexture(chartData, playResult))
	, m_topTextureRow(TopTextureRow(playResult))
{
}

void ResultPanel::draw() const
{
	using namespace ScreenUtils;

	m_renderTexture.resized(ScaledL(kPanelSize)).draw(Scaled(140) + LeftMargin(), Scaled(60));
	m_topTexture(m_topTextureRow).resized(ScaledL(kTopSize)).draw(Scaled(138) + LeftMargin(), Scaled(20));
}
