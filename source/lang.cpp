#include "lang.hpp"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <json.hpp>
#include <map>
#include <mutex>

namespace Lang {

    namespace {

        using StringMap = std::map<std::string, std::string>;

        StringMap english;
        StringMap active;
        std::string activeLanguage = "en";
        std::string langDir;

        std::mutex unknownMutex;
        std::map<std::string, std::string> unknownKeys;

        // {"a": {"b": "x"}} -> {"a.b": "x"}; the meta block is not text
        void flatten(const nlohmann::json& j, StringMap& out, const std::string& prefix = "")
        {
            for (auto it = j.begin(); it != j.end(); ++it) {
                if (prefix.empty() && it.key() == "meta")
                    continue;
                std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();
                if (it->is_object())
                    flatten(*it, out, key);
                else if (it->is_string())
                    out[key] = it->get<std::string>();
            }
        }

        bool readFile(const std::string& path, StringMap& out, LanguageInfo* info = nullptr)
        {
            std::ifstream file(path);
            if (!file.is_open())
                return false;
            try {
                nlohmann::json j = nlohmann::json::parse(file);
                if (!j.is_object())
                    return false;
                if (info && j.contains("meta") && j["meta"].is_object()) {
                    info->name = j["meta"].value("language", "");
                    info->author = j["meta"].value("author", "");
                }
                flatten(j, out);
                return true;
            }
            catch (const std::exception& e) {
                std::fprintf(stderr, "Lang: cannot parse %s: %s\n", path.c_str(), e.what());
                return false;
            }
        }

    }  // namespace

    bool init(const std::string& dir, const std::string& embeddedEnglish)
    {
        langDir = dir;
        english.clear();
        bool ok = readFile(embeddedEnglish, english);
        if (!ok)
            std::fprintf(stderr, "Lang: built-in English %s could not be loaded\n", embeddedEnglish.c_str());

        // an en.json next to the other languages overrides single keys, never removes any
        StringMap sdEnglish;
        if (readFile(langDir + "/en.json", sdEnglish)) {
            for (const auto& [key, value] : sdEnglish)
                english[key] = value;
            ok = true;
        }

        active = english;
        activeLanguage = "en";
        return ok;
    }

    std::vector<LanguageInfo> scan()
    {
        std::vector<LanguageInfo> res;
        res.push_back({"en", "English", ""});

        std::vector<LanguageInfo> found;
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(langDir, ec)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json")
                continue;
            std::string code = entry.path().stem().string();
            if (code == "en")
                continue;
            LanguageInfo info;
            info.code = code;
            StringMap unused;
            if (!readFile(entry.path().string(), unused, &info))
                continue;
            if (info.name.empty())
                info.name = code;
            found.push_back(info);
        }
        std::sort(found.begin(), found.end(), [](const LanguageInfo& a, const LanguageInfo& b) { return a.code < b.code; });
        res.insert(res.end(), found.begin(), found.end());
        return res;
    }

    bool load(const std::string& code)
    {
        if (code == "en") {
            active = english;
            activeLanguage = "en";
            return true;
        }

        // keep a language code from escaping the language directory
        if (code.empty() || code.find_first_of("/\\.") != std::string::npos)
            return false;

        StringMap loaded;
        if (!readFile(langDir + "/" + code + ".json", loaded)) {
            active = english;
            activeLanguage = "en";
            return false;
        }
        for (const auto& [key, value] : english)
            loaded.emplace(key, value);  // missing keys fall back to English

        active = std::move(loaded);
        activeLanguage = code;
        return true;
    }

    const std::string& t(const std::string& key)
    {
        auto it = active.find(key);
        if (it != active.end())
            return it->second;

        // not even in English: show the key so the gap is visible
        std::lock_guard<std::mutex> lock(unknownMutex);
        return unknownKeys.emplace(key, key).first->second;
    }

    const std::string& activeCode()
    {
        return activeLanguage;
    }

}  // namespace Lang
