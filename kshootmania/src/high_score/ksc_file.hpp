#pragma once
#include "high_score_info.hpp"
#include "ksc_key.hpp"

HighScoreInfo ReadHighScoreInfo(FilePathView kscFilePath, const KscKey& condition);
