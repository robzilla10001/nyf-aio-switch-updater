#pragma once

#include <functional>
#include <string>
#include <vector>

#include "github.hpp"

namespace release_browser {

    using AssetHandler = std::function<void(const github::Release&, const github::Asset&)>;

    // Popup with the latest `count` releases of a GitHub repo ("owner/name"). The releases are
    // listed on the left; selecting one shows its date, its downloadable files and its release
    // notes on the right. Choosing a file calls `onAsset`.
    // `extensions` limits the files shown (e.g. {"zip"}); empty shows every file.
    // If the releases can't be fetched, an explanation is shown instead of the popup.
    void open(const std::string& title, const std::string& repo, int count, const std::vector<std::string>& extensions,
              const AssetHandler& onAsset, const std::string& icon = "", const std::string& subtitle = "");

    // Text for a failed GitHub request, for dialogs
    std::string statusText(github::Status status);

}  // namespace release_browser
