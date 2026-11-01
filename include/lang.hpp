#pragma once

#include <string>
#include <vector>

// Language support modelled on GarageNX: the code only contains keys
// ("section.name"), and the text comes from a language file.
//
// English is the baseline. It is built into the app (romfs:/lang/en.json) and an
// en.json on the SD card, if present, overrides it key by key. Every other language
// is a <code>.json file in the SD language directory and falls back to English for
// any key it does not define.
namespace Lang {

    struct LanguageInfo {
        std::string code;  // file name stem: "en", "es", "pt-br", ...
        std::string name;  // meta.language, shown in the language list
        std::string author;
    };

    // `dir` holds the translations on the SD card, `embeddedEnglish` is the built-in English file.
    // Call once at startup, before any t(). Returns false if no English could be loaded at all
    // (t() then returns the keys themselves).
    bool init(const std::string& dir, const std::string& embeddedEnglish);

    // English first, then every readable *.json in the language directory, sorted by code.
    std::vector<LanguageInfo> scan();

    // Activates a language. On failure (missing/unreadable file) English stays active and false is returned.
    bool load(const std::string& code);

    // Text for a key such as "main.footer_text". Never fails: an unknown key yields the key itself.
    const std::string& t(const std::string& key);

    const std::string& activeCode();

}  // namespace Lang
