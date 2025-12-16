#include "RuntimeConfig.hpp"
#include "Ini/ConfigIni.hpp"

namespace
{
	struct Config
	{
		GaugeType gaugeType = GaugeType::kNormalGauge;
		TurnMode turnMode = TurnMode::kNormal;
		JudgmentPlayMode judgmentModeBT = JudgmentPlayMode::kOn;
		JudgmentPlayMode judgmentModeFX = JudgmentPlayMode::kOn;
		JudgmentPlayMode judgmentModeLaser = JudgmentPlayMode::kOn;
		double playbackSpeed = 1.0;
	};

	Config g_config;
}

namespace RuntimeConfig
{
	[[nodiscard]]
	GaugeType GetGaugeType() noexcept
	{
		return g_config.gaugeType;
	}

	void SetGaugeType(GaugeType gaugeType) noexcept
	{
		g_config.gaugeType = gaugeType;
	}

	[[nodiscard]]
	TurnMode GetTurnMode() noexcept
	{
		return g_config.turnMode;
	}

	void SetTurnMode(TurnMode turnMode) noexcept
	{
		g_config.turnMode = turnMode;
	}

	[[nodiscard]]
	JudgmentPlayMode GetJudgmentPlayModeBT() noexcept
	{
		return g_config.judgmentModeBT;
	}

	void SetJudgmentPlayModeBT(JudgmentPlayMode mode) noexcept
	{
		g_config.judgmentModeBT = mode;
	}

	[[nodiscard]]
	JudgmentPlayMode GetJudgmentPlayModeFX() noexcept
	{
		return g_config.judgmentModeFX;
	}

	void SetJudgmentPlayModeFX(JudgmentPlayMode mode) noexcept
	{
		g_config.judgmentModeFX = mode;
	}

	[[nodiscard]]
	JudgmentPlayMode GetJudgmentPlayModeLaser() noexcept
	{
		return g_config.judgmentModeLaser;
	}

	void SetJudgmentPlayModeLaser(JudgmentPlayMode mode) noexcept
	{
		g_config.judgmentModeLaser = mode;
	}

	[[nodiscard]]
	double GetPlaybackSpeed() noexcept
	{
		return g_config.playbackSpeed;
	}

	void SetPlaybackSpeed(double speed) noexcept
	{
		g_config.playbackSpeed = speed;
	}

	void RestoreJudgmentModesFromConfigIni()
	{
		g_config.judgmentModeBT = static_cast<JudgmentPlayMode>(ConfigIni::GetInt(ConfigIni::Key::kJudgmentModeBT, ConfigIni::Value::JudgmentMode::kOn));
		g_config.judgmentModeFX = static_cast<JudgmentPlayMode>(ConfigIni::GetInt(ConfigIni::Key::kJudgmentModeFX, ConfigIni::Value::JudgmentMode::kOn));
		g_config.judgmentModeLaser = static_cast<JudgmentPlayMode>(ConfigIni::GetInt(ConfigIni::Key::kJudgmentModeLaser, ConfigIni::Value::JudgmentMode::kOn));
	}
}
