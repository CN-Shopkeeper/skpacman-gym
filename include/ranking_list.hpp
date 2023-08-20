#pragma once

#include "pch.hpp"

class RankingList {
   public:
    class RankInfo {
       public:
        std::string id;
        std::string score;

        bool operator<(const RankInfo& other) const {
            return score < other.score;
        }

        std::string ToString() const {
            int spaces = 16 - id.length() - score.length();
            std::string result = std::string(id);
            for (int i = 0; i < spaces; i++) {
                result += " ";
            }
            result += score;
            return result;
        }
    };

    void add(const std::string& id, const int& score) {
        ranks_.Push(RankInfo{id, std::to_string(score)});
    }

    std::string ToString() const {
        std::string result = "排行榜：\n";
        for (auto& rank : ranks_) {
            result += rank.ToString() + "\n";
        }
        return result;
    }

   private:
    struct CompareGreater {
        bool operator()(const RankInfo& lhs, const RankInfo& rhs) const {
            return !(lhs < rhs);  // 降序排列
        }
    };
    FixedSizeMultiset<RankInfo, 20, CompareGreater> ranks_;
};
