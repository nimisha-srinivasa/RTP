class ScoreResult
{
public:
    int vid;
    double score;

    static bool compare(const ScoreResult &obj1, const ScoreResult &obj2);
};