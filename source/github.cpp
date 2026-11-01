#include "github.hpp"

#include <map>
#include <mutex>

#include "download.hpp"

namespace github {

    namespace {

        constexpr const char API_ROOT[] = "https://api.github.com/repos/";
        const std::vector<std::string> API_HEADERS = {"accept: application/vnd.github.v3+json"};

        std::mutex cacheMutex;
        std::map<std::string, std::vector<Release>> releasesCache;
        std::map<std::string, Release> latestCache;

        Status statusFromHttp(long code, const nlohmann::ordered_json& body)
        {
            if (code == 200)
                return Status::Ok;
            if (code == 0)
                return Status::NoNetwork;
            if (code == 404)
                return Status::NotFound;
            if (code == 429)
                return Status::RateLimited;
            // the API answers an exhausted quota with 403 and a message mentioning the rate limit
            if (code == 403 && body.is_object() && body.contains("message") && body["message"].is_string() &&
                body["message"].get<std::string>().find("rate limit") != std::string::npos)
                return Status::RateLimited;
            return Status::BadResponse;
        }

    }  // namespace

    Status getLatestRelease(const std::string& repo, Release& out)
    {
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            auto it = latestCache.find(repo);
            if (it != latestCache.end()) {
                out = it->second;
                return Status::Ok;
            }
        }

        nlohmann::ordered_json json;
        Status status = statusFromHttp(download::getRequest(API_ROOT + repo + "/releases/latest", json, API_HEADERS), json);
        if (status != Status::Ok)
            return status;
        if (!parseRelease(json, out))
            return Status::BadResponse;

        std::lock_guard<std::mutex> lock(cacheMutex);
        latestCache[repo] = out;
        return Status::Ok;
    }

    Status getReleases(const std::string& repo, std::vector<Release>& out, int count)
    {
        const std::string key = repo + "#" + std::to_string(count);
        {
            std::lock_guard<std::mutex> lock(cacheMutex);
            auto it = releasesCache.find(key);
            if (it != releasesCache.end()) {
                out = it->second;
                return Status::Ok;
            }
        }

        nlohmann::ordered_json json;
        Status status = statusFromHttp(download::getRequest(API_ROOT + repo + "/releases?per_page=" + std::to_string(count), json, API_HEADERS), json);
        if (status != Status::Ok)
            return status;
        if (!json.is_array())
            return Status::BadResponse;

        out = parseReleases(json);
        std::lock_guard<std::mutex> lock(cacheMutex);
        releasesCache[key] = out;
        return Status::Ok;
    }

}  // namespace github
