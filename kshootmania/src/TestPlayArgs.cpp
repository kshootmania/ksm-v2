#include "TestPlayArgs.hpp"

Optional<TestPlayArgs> ParseTestPlayArgs(bool* pShouldExit)
{
	*pShouldExit = false;

	const Array<String> args = System::GetCommandLineArgs();

	// args[0]は実行ファイルパス、args[1]以降がユーザー引数
	if (args.size() <= 1)
	{
		return none;
	}

	TestPlayArgs result;

	for (size_t i = 1; i < args.size(); ++i)
	{
		const String& arg = args[i];

		if (arg == U"-auto")
		{
			result.isAutoPlay = MusicGame::IsAutoPlayYN::Yes;
		}
		else if (arg == U"-hard")
		{
			result.testPlayOption.gaugeType = GaugeType::kHardGauge;
		}
		else if (arg == U"-easy")
		{
			result.testPlayOption.gaugeType = GaugeType::kEasyGauge;
		}
		else if (arg == U"-from" && i + 1 < args.size())
		{
			++i;
			const auto measureOpt = ParseIntOpt<int32>(args[i]);
			if (measureOpt.has_value())
			{
				result.testPlayOption.startMeasure = Max(*measureOpt, 0);
			}
		}
		else if (FsUtils::HasChartExtension(arg))
		{
			result.chartFilePath = FilePath{ arg };
		}
	}

	// 譜面ファイルが指定されていない場合は通常起動
	if (result.chartFilePath.isEmpty())
	{
		return none;
	}

	// ファイルが存在しない場合はエラー終了
	if (!FileSystem::Exists(result.chartFilePath))
	{
		System::MessageBoxOK(U"Chart file not found:\n" + result.chartFilePath, MessageBoxStyle::Error);
		*pShouldExit = true;
		return none;
	}

	return result;
}
