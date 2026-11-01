#pragma once

#include <cctype>
#include <string>

namespace util {

    namespace version_detail {

        // '.' ranks above letters so that a point release beats a letter revision:
        // 3.0a < 3.0.1
        inline int compareSeparators(const std::string& a, size_t i, size_t ei, const std::string& b, size_t j, size_t ej)
        {
            auto rank = [](char c) { return c == '.' ? 256 : static_cast<unsigned char>(c); };
            while (i < ei && j < ej) {
                int ra = rank(a[i++]);
                int rb = rank(b[j++]);
                if (ra != rb)
                    return ra > rb ? 1 : -1;
            }
            if (i < ei) return 1;
            if (j < ej) return -1;
            return 0;
        }

        // Splits a version into runs of digits and runs of non-digits and compares
        // them pairwise: digit runs numerically, other runs lexically. When one
        // version is a prefix of the other, the longer one is newer, so
        // 3.0 < 3.0a < 3.0b < 3.0.1 < 3.1 < 3.10
        inline int compare(const std::string& a, const std::string& b)
        {
            auto skipPrefix = [](const std::string& s) {
                return (!s.empty() && (s[0] == 'v' || s[0] == 'V')) ? 1u : 0u;
            };
            size_t i = skipPrefix(a);
            size_t j = skipPrefix(b);

            while (i < a.size() && j < b.size()) {
                bool da = std::isdigit(static_cast<unsigned char>(a[i]));
                bool db = std::isdigit(static_cast<unsigned char>(b[j]));
                if (da != db)
                    return da ? 1 : -1;

                size_t ei = i;
                size_t ej = j;
                if (da) {
                    while (ei < a.size() && std::isdigit(static_cast<unsigned char>(a[ei]))) ei++;
                    while (ej < b.size() && std::isdigit(static_cast<unsigned char>(b[ej]))) ej++;
                    // numeric compare without overflow: strip leading zeros, then length, then lexical
                    size_t si = i;
                    size_t sj = j;
                    while (si + 1 < ei && a[si] == '0') si++;
                    while (sj + 1 < ej && b[sj] == '0') sj++;
                    if ((ei - si) != (ej - sj))
                        return (ei - si) > (ej - sj) ? 1 : -1;
                    int c = a.compare(si, ei - si, b, sj, ej - sj);
                    if (c != 0)
                        return c > 0 ? 1 : -1;
                }
                else {
                    while (ei < a.size() && !std::isdigit(static_cast<unsigned char>(a[ei]))) ei++;
                    while (ej < b.size() && !std::isdigit(static_cast<unsigned char>(b[ej]))) ej++;
                    int c = compareSeparators(a, i, ei, b, j, ej);
                    if (c != 0)
                        return c;
                }
                i = ei;
                j = ej;
            }

            if (i < a.size()) return 1;
            if (j < b.size()) return -1;
            return 0;
        }

    }  // namespace version_detail

    // true only if `candidate` is strictly newer than `current`
    inline bool isNewerVersion(const std::string& candidate, const std::string& current)
    {
        return !candidate.empty() && version_detail::compare(candidate, current) > 0;
    }

}  // namespace util
