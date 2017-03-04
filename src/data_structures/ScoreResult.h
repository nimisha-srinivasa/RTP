#ifndef _SCORERESULT_H_
#define _SCORERESULT_H_

class ScoreResult
{
public:
    int vid;
    double score;

    static bool compare(const ScoreResult &obj1, const ScoreResult &obj2);
};

#endif