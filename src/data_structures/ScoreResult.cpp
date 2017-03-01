#include "ScoreResult.h"

bool ScoreResult::compare(const ScoreResult &obj1, const ScoreResult &obj2){
	//for arranging in descending order of scores
	return (obj1.score > obj2.score);
}