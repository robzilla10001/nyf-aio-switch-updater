#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <json.hpp>
#include <string>
#include <vector>

namespace github {

    enum class Status {
        Ok,
        NoNetwork,    // no response at all (offline, DNS, timeout)
        RateLimited,  // unauthenticated API quota (60/h) used up
        NotFound,     // unknown repo or no releases (for /latest)
        BadResponse,  // anything else
    };

    struct Asset {
        std::string name;
        std::string url;  // browser_download_url, directly downloadable
        std::uint64_t size = 0;
    };

    struct Release {
        std::string tag;
        std::string name;
        std::string body;       // markdown release notes
        std::string published;  // ISO 8601
        bool prerelease = false;
        std::vector<Asset> assets;
    };

    namespace detail {

        inline std::string lower(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
            return s;
        }

        inline bool endsWith(const std::string& s, const std::string& suffix)
        {
            return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        inline std::string stringOr(const nlohmann::ordered_json& j, const char* key)
        {
            return (j.contains(key) && j[key].is_string()) ? j[key].get<std::string>() : "";
        }

    }  // namespace detail

    // Fills `out` from one GitHub release object. Drafts and objects without a tag are rejected.
    inline bool parseRelease(const nlohmann::ordered_json& j, Release& out)
    {
        if (!j.is_object() || j.value("draft", false) || detail::stringOr(j, "tag_name").empty())
            return false;

        out = Release();
        out.tag = detail::stringOr(j, "tag_name");
        out.name = detail::stringOr(j, "name");
        out.body = detail::stringOr(j, "body");
        out.published = detail::stringOr(j, "published_at");
        out.prerelease = j.value("prerelease", false);

        if (j.contains("assets") && j["assets"].is_array()) {
            for (const auto& a : j["assets"]) {
                Asset asset;
                asset.name = detail::stringOr(a, "name");
                asset.url = detail::stringOr(a, "browser_download_url");
                asset.size = a.contains("size") && a["size"].is_number_unsigned() ? a["size"].get<std::uint64_t>() : 0;
                if (!asset.name.empty() && !asset.url.empty())
                    out.assets.push_back(asset);
            }
        }
        return true;
    }

    // Parses a releases list, skipping drafts. A non-array (an API error object) yields an empty list.
    inline std::vector<Release> parseReleases(const nlohmann::ordered_json& j)
    {
        std::vector<Release> res;
        if (j.is_array()) {
            for (const auto& item : j) {
                Release release;
                if (parseRelease(item, release))
                    res.push_back(std::move(release));
            }
        }
        return res;
    }

    // `spec` is either an exact file name ("KitNX.bin", case-insensitive) or "{first <ext>}",
    // e.g. "{first zip}" / "{first 7z}", meaning the first asset with that extension in the
    // order GitHub lists them. Returns nullptr when nothing matches.
    inline const Asset* findAsset(const Release& release, const std::string& spec)
    {
        constexpr const char prefix[] = "{first ";
        if (spec.size() > sizeof(prefix) && spec.compare(0, sizeof(prefix) - 1, prefix) == 0 && spec.back() == '}') {
            std::string ext = "." + detail::lower(spec.substr(sizeof(prefix) - 1, spec.size() - sizeof(prefix)));
            for (const auto& asset : release.assets) {
                if (detail::endsWith(detail::lower(asset.name), ext))
                    return &asset;
            }
            return nullptr;
        }
        std::string wanted = detail::lower(spec);
        for (const auto& asset : release.assets) {
            if (detail::lower(asset.name) == wanted)
                return &asset;
        }
        return nullptr;
    }

    inline std::string releasesPageUrl(const std::string& repo)
    {
        return "https://github.com/" + repo + "/releases";
    }

    // ---- network (github.cpp) ----------------------------------------------------------

    // Latest published, non-prerelease release of `repo` ("owner/name").
    Status getLatestRelease(const std::string& repo, Release& out);

    // Up to `count` most recent non-draft releases (prereleases included), newest first.
    // Results are cached for the lifetime of the app so that opening several tabs does not
    // burn through the unauthenticated API quota; only successful lookups are cached.
    Status getReleases(const std::string& repo, std::vector<Release>& out, int count = 10);

}  // namespace github
