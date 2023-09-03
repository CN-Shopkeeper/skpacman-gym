#pragma once

#include "pch.hpp"

class RankingList : public Singlton<RankingList> {
   public:
    class RankInfo {
       public:
        std::string id;
        std::string score;

        bool operator<(const RankInfo& other) const {
            return std::stoi(score) < std::stoi(other.score);
        }

        std::string ToString() const {
            int spaces = 18 - id.length() - score.length();
            std::string result = std::string(id);
            for (int i = 0; i < spaces; i++) {
                result += " ";
            }
            result += score;
            return result;
        }
    };

    RankingList() { getRankingListFromFile(); }

    ~RankingList() { delete postChar; }

    void add(const std::string& id, const int& score) {
        writeToFile(RankInfo{id, std::to_string(score)});
    }

    void Clear() { ranks_.Clear(); }

    std::string ToString() const {
        std::string result = "RankingList(Top30)\n";
        for (auto& rank : ranks_) {
            result += rank.ToString() + "\n";
        }
        return result;
    }

    auto& getRanks() { return ranks_; }

   private:
    struct CompareGreater {
        bool operator()(const RankInfo& lhs, const RankInfo& rhs) const {
            return !(lhs < rhs);  // 降序排列
        }
    };
    FixedSizeMultiset<RankInfo, 30, CompareGreater> ranks_;
    inline static char* postChar;

    inline static const char* const requestHeaders[3] = {
        "Content-Type", "application/json", nullptr};

    void getRankingListFromFile() {
        std::ifstream inFile("pacman_ranking_list.txt");
        if (!inFile.is_open()) {
            // 文件打开失败，您可以选择在此处创建文件
            std::ofstream output("pacman_ranking_list.txt");
            if (output.is_open()) {
                output.close();
            } else {
                // 文件创建失败
                std::cerr << "无法创建文件 pacman_ranking_list.txt"
                          << std::endl;
            }
        } else {
            // 文件成功打开，您可以在这里读取文件的内容
            std::string id;
            std::string score;
            while (std::getline(inFile, id) && std::getline(inFile, score)) {
                ranks_.Push({id, score});
            }
            inFile.close();
        }
    }

    void writeToFile(const RankInfo& record) {
        ranks_.Push(record);
        // 写入文本文件，如果文件不存在则创建它
        std::ofstream outFile("pacman_ranking_list.txt");
        if (outFile.is_open()) {
            for (auto& rank : ranks_) {
                outFile << rank.id << std::endl;
                outFile << rank.score << std::endl;
            }
            outFile.close();
            std::cout << "Ranking record written successfully." << std::endl;
        } else {
            std::cerr << "Failed to open file for Ranking List." << std::endl;
        }
    }
};
