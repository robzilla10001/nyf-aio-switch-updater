#pragma once

#include <borealis.hpp>
#include <json.hpp>

#include "constants.hpp"

class AmsTab : public brls::List
{
protected:
    brls::ListItem* listItem;
    bool erista;
    nlohmann::ordered_json hekate;  // {file name: url} of the latest hekate release, empty if unavailable
    contentType type;
    void CreateStagedFrames(const std::string& text, const std::string& url, bool erista, bool ams = true, bool hekate = false, const std::string& text_hekate = "", const std::string& hekate_url = "");
    // Confirm, download, extract (and optionally offer hekate) for one archive
    void StartInstall(const std::string& name, const std::string& url, bool offerHekate, bool ams = true);
    bool CreateDownloadItems(const nlohmann::ordered_json& cfw_links, bool hekate = true, bool ams = true);
    void CreateNotFoundLabel();
    virtual void RegisterListItemAction(brls::ListItem* listItem);

public:
    AmsTab(const bool erista = true);
};

class AmsTab_Regular : public AmsTab
{
private:
    void CreateLists();
    bool CreateDownloadItems(const nlohmann::ordered_json& cfw_links, bool hekate = true, bool ams = true);
    nlohmann::ordered_json GetLatestAtmosphere();
    void ShowBornToHonkReleases();

public:
    AmsTab_Regular(const bool erista = true);
};

class AmsTab_Custom : public AmsTab
{
private:
    nlohmann::ordered_json custom_packs;
    void CreateLists();
    void RegisterListItemAction(brls::ListItem* listItem) override;
    void AddLinkCreator();

public:
    AmsTab_Custom(const bool erista = true);
};
