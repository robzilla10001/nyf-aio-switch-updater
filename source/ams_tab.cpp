#include "ams_tab.hpp"

#include <filesystem>
#include <iostream>
#include <string>

#include "confirm_page.hpp"
#include "current_cfw.hpp"
#include "dialogue_page.hpp"
#include "download.hpp"
#include "extract.hpp"
#include "fs.hpp"
#include "github.hpp"
#include "release_browser.hpp"
#include "utils.hpp"
#include "worker_page.hpp"
#include "lang.hpp"
namespace i18n = brls::i18n;
using namespace i18n::literals;

AmsTab::AmsTab(const bool erista) : brls::List()
{
    this->erista = erista;

    // hekate is offered after installing a pack that does not include it
    github::Release release;
    if (github::getLatestRelease(HEKATE_REPO, release) == github::Status::Ok) {
        if (const github::Asset* zip = github::findAsset(release, "{first zip}"))
            this->hekate[zip->name] = zip->url;
    }
}

void AmsTab::RegisterListItemAction(brls::ListItem* listItem) {}

void AmsTab::StartInstall(const std::string& name, const std::string& url, bool offerHekate, bool ams)
{
    if (!erista && !std::filesystem::exists(MARIKO_PAYLOAD_PATH)) {
        brls::Application::crash(Lang::t("errors.mariko_payload_missing"));
        return;
    }

    std::string text(Lang::t("common.download") + name + Lang::t("common.from") + url);
    bool withHekate = offerHekate && !this->hekate.empty();
    std::string text_hekate, hekate_url;
    if (withHekate) {
        auto hekate_link = download::getLinksFromJson(this->hekate);
        hekate_url = hekate_link[0].second;
        text_hekate = Lang::t("common.download") + hekate_link[0].first;
    }
    CreateStagedFrames(text, url, erista, ams, withHekate, text_hekate, hekate_url);
}

bool AmsTab::CreateDownloadItems(const nlohmann::ordered_json& cfw_links, bool hekate, bool ams)
{
    std::vector<std::pair<std::string, std::string>> links = download::getLinksFromJson(cfw_links);
    if (links.empty())
        return false;

    for (const auto& link : links) {
        bool pack = link.first.contains("[PACK]");
        std::string name = link.first;
        std::string url = link.second;
        listItem = new brls::ListItem(name);
        listItem->setHeight(LISTITEM_HEIGHT);
        listItem->getClickEvent()->subscribe([this, name, url, hekate, pack, ams](brls::View* view) {
            StartInstall(name, url, hekate && !pack, ams);
        });
        this->RegisterListItemAction(listItem);
        this->addView(listItem);
    }
    return true;
}

void AmsTab::CreateStagedFrames(const std::string& text, const std::string& url, bool erista, bool ams, bool hekate, const std::string& text_hekate, const std::string& hekate_url)
{
    brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
    stagedFrame->setTitle(this->type == contentType::ams_cfw ? Lang::t("ams_update.getting_ams") : Lang::t("ams_update.custom_download"));
    stagedFrame->addStage(
        new ConfirmPage(stagedFrame, text));
    stagedFrame->addStage(
        new WorkerPage(stagedFrame, Lang::t("common.downloading"), [&, url]() { util::downloadArchive(url, this->type); }));
    stagedFrame->addStage(
        new WorkerPage(stagedFrame, Lang::t("common.extracting"), [&]() { util::extractArchive(this->type); }));
    if (hekate) {
        stagedFrame->addStage(
            new DialoguePage_ams(stagedFrame, text_hekate, erista));
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, Lang::t("common.downloading"), [hekate_url]() { util::downloadArchive(hekate_url, contentType::bootloaders); }));
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, Lang::t("common.extracting"), []() { util::extractArchive(contentType::bootloaders); }));
    }
    if (ams)
        stagedFrame->addStage(new ConfirmPage_AmsUpdate(stagedFrame, Lang::t("ams_update.reboot_rcm"), erista));
    else
        stagedFrame->addStage(new ConfirmPage_Done(stagedFrame, Lang::t("common.all_done")));
    brls::Application::pushView(stagedFrame);
}

AmsTab_Regular::AmsTab_Regular(const bool erista) : AmsTab(erista)
{
    this->CreateLists();
}

bool AmsTab_Regular::CreateDownloadItems(const nlohmann::ordered_json& cfw_links, bool hekate, bool ams)
{
    if (!AmsTab::CreateDownloadItems(cfw_links, hekate, ams)) {
        brls::Label* description = new brls::Label(
            brls::LabelStyle::SMALL,
            Lang::t("main.links_not_found"),
            true);
        description->setHorizontalAlign(NVG_ALIGN_CENTER);
        this->addView(description);
        return true;
    }
    return false;
}

nlohmann::ordered_json AmsTab_Regular::GetLatestAtmosphere()
{
    nlohmann::ordered_json links = nlohmann::ordered_json::object();
    github::Release release;
    if (github::getLatestRelease(AMS_REPO, release) == github::Status::Ok) {
        if (const github::Asset* zip = github::findAsset(release, "{first zip}"))
            links[zip->name] = zip->url;
    }
    return links;
}

void AmsTab_Regular::ShowBornToHonkReleases()
{
    release_browser::open(
        Lang::t("ams_update.bth_fork_title"), BTH_REPO, 10, {"zip"},
        [this](const github::Release& release, const github::Asset& asset) {
            this->StartInstall(asset.name, asset.url, true);
        },
        "romfs:/bth_icon.png", github::releasesPageUrl(BTH_REPO));
}

void AmsTab_Regular::CreateLists()
{
    this->type = contentType::ams_cfw;

    this->addView(new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("main.ams_text") + "\n" + Lang::t("ams_update.current_ams") + CurrentCfw::getAmsInfo() + (erista ? "\n" + Lang::t("ams_update.erista_rev") : "\n" + Lang::t("ams_update.mariko_rev")), true));
    CreateDownloadItems(GetLatestAtmosphere());

    this->addView(new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        Lang::t("ams_update.bth_label"),
        true));
    listItem = new brls::ListItem(Lang::t("ams_update.download_bth_fork"));
    listItem->setHeight(LISTITEM_HEIGHT);
    listItem->getClickEvent()->subscribe([this](brls::View* view) {
        this->ShowBornToHonkReleases();
    });
    this->addView(listItem);
}

AmsTab_Custom::AmsTab_Custom(const bool erista) : AmsTab(erista)
{
    this->custom_packs = fs::parseJsonFile(CUSTOM_PACKS_PATH);
    this->CreateLists();
}

void AmsTab_Custom::CreateLists()
{
    this->addView(new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        fmt::format(Lang::t("ams_update.custom_packs_label"), CUSTOM_PACKS_PATH),
        true));

    this->type = contentType::ams_cfw;
    this->addView(new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        Lang::t("ams_update.custom_packs_ams"),
        true));
    CreateDownloadItems(util::getValueFromKey(this->custom_packs, "ams"), true);
    this->AddLinkCreator();

    this->type = contentType::custom;
    this->addView(new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        Lang::t("ams_update.custom_packs_misc"),
        true));
    CreateDownloadItems(util::getValueFromKey(this->custom_packs, "misc"), false, false);
    this->AddLinkCreator();
}

void AmsTab_Custom::AddLinkCreator()
{
    std::string category = this->type == contentType::ams_cfw ? "ams" : "misc";
    listItem = new brls::ListItem(Lang::t("ams_update.add_custom_link"));
    listItem->setHeight(LISTITEM_HEIGHT);
    listItem->getClickEvent()->subscribe([this, category](brls::View* view) {
        std::string title, link;
        brls::Swkbd::openForText([&title](std::string text) { title = text; }, "Enter title", "", 256, "", 0, "Submit", "Title");
        brls::Swkbd::openForText([&link](std::string text) { link = text; }, "Enter direct link", "", 256, "", 0, "Submit", "https://site/download.zip");
        auto links = util::getValueFromKey(this->custom_packs, category);
        links[title] = link;
        this->custom_packs[category] = links;
        fs::writeJsonToFile(this->custom_packs, CUSTOM_PACKS_PATH);
        util::restartApp();
    });
    this->addView(listItem);
}

void AmsTab_Custom::RegisterListItemAction(brls::ListItem* listItem)
{
    std::string label = listItem->getLabel();
    std::string category = this->type == contentType::ams_cfw ? "ams" : "misc";
    listItem->registerAction(Lang::t("ams_update.delete_custom_link"), brls::Key::X, [this, label, category] {
        auto& links = this->custom_packs.at(category);
        links.erase(label);
        fs::writeJsonToFile(this->custom_packs, CUSTOM_PACKS_PATH);
        util::restartApp();
        return true;
    });
}
