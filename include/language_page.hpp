#pragma once

#include <borealis.hpp>
#include <functional>
#include <string>

namespace language {

    // Applies the language stored in LANGUAGE_JSON. Returns false if that settings file is
    // missing or unreadable, i.e. the user has not chosen a language yet. A stored language
    // whose file is gone is not an error: English stays active.
    bool loadSaved();

    void save(const std::string& code);

    // List of the available languages; `onSelected` receives the chosen language code.
    brls::AppletFrame* createPicker(const std::function<void(const std::string& code)>& onSelected);

}  // namespace language
