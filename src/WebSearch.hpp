#pragma once
#include <string>
#include <vector>
#include <functional>
struct SearchResult { std::string title, url, content; };
class WebSearch {
public:
    static WebSearch* get();
    void search(const std::string& q,
        std::function<void(std::vector<SearchResult>)> ok,
        std::function<void(std::string)> err);
    static std::string formatForPrompt(const std::vector<SearchResult>& r);
    bool isAvailable();
};
