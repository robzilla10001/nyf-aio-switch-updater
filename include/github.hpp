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

    // Release notes are markdown; the UI can only show plain text. This drops images and
    // markup (including HTML tags), keeps link texts, turns list markers into "- " (nested ones indented) and
    // collapses runs of blank lines.
    inline std::string plainText(const std::string& markdown)
    {
        std::string s;
        for (char c : markdown) {  // normalise line endings
            if (c != '\r')
                s += c;
        }

        auto replaceAll = [](std::string& str, const std::string& from, const std::string& to) {
            for (size_t pos = 0; (pos = str.find(from, pos)) != std::string::npos; pos += to.size())
                str.replace(pos, from.size(), to);
        };

        // images ![alt](url) vanish, links [text](url) keep their text
        for (size_t pos = 0; (pos = s.find('[', pos)) != std::string::npos;) {
            size_t close = s.find(']', pos);
            if (close == std::string::npos || close + 1 >= s.size() || s[close + 1] != '(') {
                pos++;
                continue;
            }
            size_t end = s.find(')', close);
            if (end == std::string::npos || s.find('\n', close) < end) {
                pos++;
                continue;
            }
            bool image = pos > 0 && s[pos - 1] == '!';
            std::string text = image ? "" : s.substr(pos + 1, close - pos - 1);
            size_t from = image ? pos - 1 : pos;
            s.replace(from, end - from + 1, text);
            pos = from + text.size();
        }

        // HTML tags and comments (<details>, </summary>, <!-- ... -->) are dropped, their content stays
        for (size_t pos = 0; (pos = s.find('<', pos)) != std::string::npos;) {
            size_t end = s.find('>', pos);
            bool tag = end != std::string::npos && end - pos < 120 && pos + 1 < s.size() &&
                       (std::isalpha(static_cast<unsigned char>(s[pos + 1])) || s[pos + 1] == '/' || s[pos + 1] == '!');
            if (tag)
                s.erase(pos, end - pos + 1);
            else
                pos++;
        }

        replaceAll(s, "**", "");
        replaceAll(s, "__", "");
        replaceAll(s, "`", "");

        std::string out;
        size_t blank = 0;
        size_t start = 0;
        while (start <= s.size()) {
            size_t nl = s.find('\n', start);
            std::string line = s.substr(start, nl == std::string::npos ? std::string::npos : nl - start);
            start = nl == std::string::npos ? s.size() + 1 : nl + 1;

            size_t indent = line.find_first_not_of(' ');
            if (indent == std::string::npos) {
                if (++blank <= 1 && !out.empty())
                    out += '\n';
                continue;
            }
            std::string body = line.substr(indent);
            if (body.size() > 1 && (body[0] == '*' || body[0] == '-' || body[0] == '+') && body[1] == ' ')
                body = std::string(indent >= 2 ? 2 : 0, ' ') + "- " + body.substr(2);
            else if (body[0] == '#')
                body = body.substr(body.find_first_not_of("# "));
            else if (body.find_first_not_of("-=_ ") == std::string::npos)
                continue;  // horizontal rules / heading underlines
            blank = 0;
            out += body + '\n';
        }
        while (!out.empty() && out.back() == '\n')
            out.pop_back();
        return out;
    }

    // Splits text at blank lines. Stops after roughly `maxChars` and says so with an ellipsis.
    inline std::vector<std::string> splitParagraphs(const std::string& text, size_t maxChars = 6000)
    {
        std::vector<std::string> res;
        size_t total = 0;
        size_t start = 0;
        while (start < text.size()) {
            size_t end = text.find("\n\n", start);
            std::string paragraph = text.substr(start, end == std::string::npos ? std::string::npos : end - start);
            start = end == std::string::npos ? text.size() : end + 2;
            if (paragraph.empty())
                continue;
            if (total + paragraph.size() > maxChars) {
                res.push_back("...");
                break;
            }
            total += paragraph.size();
            res.push_back(paragraph);
        }
        return res;
    }

    // "2026-10-16T00:05:51Z" -> "2026-10-16"
    inline std::string shortDate(const std::string& iso)
    {
        return iso.substr(0, 10);
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
