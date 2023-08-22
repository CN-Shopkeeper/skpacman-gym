#pragma once

#include "pch.hpp"
// ! 这个文件中存储了server的URL，被.gitignore忽略
#include "server_info.hpp"

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
#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        // strcpy(attr.requestUrl,
        //        "http://146.56.248.15/games_server/pacman/ranking_list");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.onsuccess = downloadSucceeded;
        attr.onerror = downloadFailed;
        emscripten_fetch(&attr, UrlGet.data());
#else
        std::ifstream inFile("pacman_ranking_list.txt");
        if (inFile.is_open()) {
            std::string id;
            std::string score;
            while (std::getline(inFile, id) && std::getline(inFile, score)) {
                ranks_.Push({id, score});
            }
            inFile.close();
        } else {
            std::cerr << "Failed to open file for reading." << std::endl;
        }
#endif
    }

    void writeToFile(const RankInfo& record) {
#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>

        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        // 配置请求属性
        strcpy(attr.requestMethod, "POST");
        // JSON 数据作为请求体
        std::string jsonBody = "{\"id\": \"" + record.id + "\",\"score\": \"" +
                               record.score + "\"}";

        // 设置POST请求的JSON数据
        postChar = new char[jsonBody.size() + 1];
        std::copy(jsonBody.begin(), jsonBody.end(), postChar);
        postChar[jsonBody.size()] = '\0';
        attr.requestData = postChar;
        attr.requestDataSize = strlen(postChar);

        attr.requestHeaders = requestHeaders;

        // 配置回调函数
        attr.onsuccess = onRequestComplete;
        attr.onerror = onRequestError;

        // 发起异步请求
        emscripten_fetch(&attr, UrlPost.data());

#else
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
#endif
    }

#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
    static void downloadSucceeded(emscripten_fetch_t* fetch) {
        printf("Finished downloading %llu bytes for Ranking List.\n",
               fetch->numBytes);
        // The data is now available at fetch->data[0] through
        // fetch->data[fetch->numBytes-1];
        auto& rankingList = RankingList::GetInstance();
        std::istringstream stream(std::string(fetch->data, fetch->numBytes));
        std::string id;
        std::string score;
        while (std::getline(stream, id) && std::getline(stream, score)) {
            rankingList.getRanks().Push({id, score});
        }
        emscripten_fetch_close(fetch);  // Free data associated with the fetch.
    }

    static void downloadFailed(emscripten_fetch_t* fetch) {
        printf(
            "Downloading Ranking List failed, HTTP failure status code: %d.\n",
            fetch->status);
        emscripten_fetch_close(fetch);  // Also free data on failure.
    }

    // 定义异步请求完成后的回调函数
    static void onRequestComplete(emscripten_fetch_t* fetch) {
        if (fetch->status == 200) {
            auto& rankingList = RankingList::GetInstance();
            // 清空，重新赋值
            rankingList.Clear();
            // 将 fetch->data 转换为字符串
            std::istringstream stream(
                std::string(fetch->data, fetch->numBytes));
            std::string id;
            std::string score;
            while (std::getline(stream, id) && std::getline(stream, score)) {
                rankingList.getRanks().Push({id, score});
            }
        } else {
            printf("Request failed with status: %d\n", fetch->status);
        }

        emscripten_fetch_close(fetch);
        delete postChar;
    }

    static void onRequestError(emscripten_fetch_t* fetch) {
        printf(
            "Uploading Ranking record failed, HTTP failure status code: %d.\n",
            fetch->status);
        emscripten_fetch_close(fetch);  // Also free data on failure.
        delete postChar;
    }
#endif
};
