#pragma once
#include "high_score_info.hpp"
#include "high_score_condition.hpp"

HighScoreInfo ReadHighScoreInfo(FilePathView kscFilePath, const HighScoreCondition& condition);
