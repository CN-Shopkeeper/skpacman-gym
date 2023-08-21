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

    inline static RankInfo toAdd_;

    RankingList() { getRankingListFromFile(); }

    ~RankingList() { delete postChar; }

    void add(const std::string& id, const int& score) {
        // 先存下来，等更新一波排行榜后再写入
        RankingList::toAdd_ = RankInfo{id, std::to_string(score)};
        writeToFile();
    }

    void Clear() { ranks_.Clear(); }

    std::string ToString() const {
        std::string result = "排行榜：\n";
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
    FixedSizeMultiset<RankInfo, 20, CompareGreater> ranks_;
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
        emscripten_fetch(&attr, Url.data());
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

    void writeToFile() {
#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
        // 先拉取更新排行榜
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);
        strcpy(attr.requestMethod, "GET");
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.onsuccess = startUpload;
        attr.onerror = downloadFailed;
        emscripten_fetch(&attr, Url.data());

#else
        ranks_.Push(RankingList::toAdd_);
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

    static void startUpload(emscripten_fetch_t* fetch) {
        printf("Finished downloading %llu bytes for Ranking List.\n",
               fetch->numBytes);
        auto& rankingList = RankingList::GetInstance();
        // 清空，重新赋值
        rankingList.Clear();
        std::istringstream stream(std::string(fetch->data, fetch->numBytes));
        std::string id;
        std::string score;
        while (std::getline(stream, id) && std::getline(stream, score)) {
            rankingList.getRanks().Push({id, score});
        }
        rankingList.getRanks().Push(RankingList::toAdd_);
        emscripten_fetch_close(fetch);  // Free data associated with the fetch.

        std::cout << "try upload" << std::endl;
        emscripten_fetch_attr_t attr;
        emscripten_fetch_attr_init(&attr);

        // 配置请求属性
        strcpy(attr.requestMethod, "POST");

        auto& ranks_ = rankingList.getRanks();
        // JSON 数据作为请求体
        std::string jsonBody = "[";
        for (auto it = ranks_.begin(); it != ranks_.end(); it++) {
            auto& rank = *it;
            if (it == ranks_.begin()) {
                jsonBody += "{\"id\": \"" + rank.id + "\",\"score\": \"" +
                            rank.score + "\"}";
            } else {
                jsonBody += ",{\"id\": \"" + rank.id + "\",\"score\": \"" +
                            rank.score + "\"}";
            }
        }
        jsonBody += "]";

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
        emscripten_fetch(&attr, Url.data());
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
            std::string responseData((char*)fetch->data, fetch->numBytes);
            printf("Upload rank record success!\nResponse: %s\n",
                   responseData.c_str());
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
