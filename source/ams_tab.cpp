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
#include "utils.hpp"
#include "worker_page.hpp"
#include "lang.hpp"
namespace i18n = brls::i18n;
using namespace i18n::literals;

AmsTab::AmsTab(const nlohmann::ordered_json& nxlinks, const bool erista) : brls::List()
{
    this->erista = erista;
    this->nxlinks = nxlinks;
    this->hekate = util::getValueFromKey(nxlinks, "hekate");
}

void AmsTab::RegisterListItemAction(brls::ListItem* listItem) {}

bool AmsTab::CreateDownloadItems(const nlohmann::ordered_json& cfw_links, bool hekate, bool ams)
{
    std::vector<std::pair<std::string, std::string>> links;
    links = download::getLinksFromJson(cfw_links);
    if (links.size() && !this->hekate.empty()) {  // non-empty this->hekate indicates internet connection
        auto hekate_link = download::getLinksFromJson(this->hekate);
        std::string hekate_url = hekate_link[0].second;
        std::string text_hekate = Lang::t("common.download") + hekate_link[0].first;

        for (const auto& link : links) {
            bool pack = link.first.contains("[PACK]");
            std::string url = link.second;
            std::string text(Lang::t("common.download") + link.first + Lang::t("common.from") + url);
            listItem = new brls::ListItem(link.first);
            listItem->setHeight(LISTITEM_HEIGHT);
            listItem->getClickEvent()->subscribe([this, text, text_hekate, url, hekate_url, hekate, pack, ams](brls::View* view) {
                if (!erista && !std::filesystem::exists(MARIKO_PAYLOAD_PATH)) {
                    brls::Application::crash(Lang::t("errors.mariko_payload_missing"));
                }
                else {
                    CreateStagedFrames(text, url, erista, ams, hekate && !pack, text_hekate, hekate_url);
                }
            });
            this->RegisterListItemAction(listItem);
            this->addView(listItem);
        }
        return true;
    }
    return false;
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

AmsTab_Regular::AmsTab_Regular(const nlohmann::ordered_json& nxlinks, const bool erista) : AmsTab(nxlinks, erista)
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

void AmsTab_Regular::CreateLists()
{
    this->type = contentType::ams_cfw;
    auto cfws = util::getValueFromKey(this->nxlinks, "cfws");

    this->addView(new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("main.ams_text") + "\n" + Lang::t("ams_update.current_ams") + CurrentCfw::getAmsInfo() + (erista ? "\n" + Lang::t("ams_update.erista_rev") : "\n" + Lang::t("ams_update.mariko_rev")), true));
    CreateDownloadItems(util::getValueFromKey(cfws, "Atmosphere"));

    this->addView(new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        Lang::t("ams_update.deepsea_label"),
        true));
    listItem = new brls::ListItem(Lang::t("ams_update.get_custom_deepsea"));
    listItem->setHeight(LISTITEM_HEIGHT);
    listItem->getClickEvent()->subscribe([this](brls::View* view) {
        nlohmann::ordered_json modules;
        download::getRequest(DEEPSEA_META_JSON, modules);
        this->ShowCustomDeepseaBuilder(modules);
    });
    this->addView(listItem);
    CreateDownloadItems(util::getValueFromKey(cfws, "DeepSea"), false);
}

std::string AmsTab_Regular::GetRepoName(const std::string& repo)
{
    return repo.substr(repo.find("/") + 1, repo.length());
}

std::set<std::string> AmsTab_Regular::GetLastDownloadedModules(const std::string& json_path)
{
    nlohmann::ordered_json package = fs::parseJsonFile(json_path);
    std::set<std::string> res;
    if (package.find("modules") != package.end()) {
        for (const auto& module : package.at("modules")) {
            res.insert(module.get<std::string>());
        }
    }
    return res;
}

nlohmann::ordered_json AmsTab_Regular::SortDeepseaModules(const nlohmann::ordered_json& modules)
{
    nlohmann::ordered_json sorted_modules = nlohmann::ordered_json::object();
    if (modules.find("modules") != modules.end()) {
        for (const auto& module : modules.at("modules").items()) {
            sorted_modules[std::string(module.value().at("category"))][module.key()] = module.value();
        }
    }
    return sorted_modules;
}

void AmsTab_Regular::ShowCustomDeepseaBuilder(nlohmann::ordered_json& modules)
{
    modules = SortDeepseaModules(modules);
    std::map<std::string, std::string> name_map;

    brls::TabFrame* appView = new brls::TabFrame();
    appView->setIcon("romfs:/deepsea_icon.png");

    std::vector<brls::List*> lists;
    std::set<std::string> old_modules = GetLastDownloadedModules(DEEPSEA_PACKAGE_PATH);

    brls::ToggleListItem* deepseaListItem;
    for (const auto& category : modules.items()) {
        brls::List* list = new brls::List();

        for (const auto& module : category.value().items()) {
            auto module_value = module.value();
            std::string requirements = "";
            if (!module_value.at("requires").empty()) {
                requirements = Lang::t("ams_update.depends_on");
                for (const auto& r : module.value().at("requires")) {
                    requirements += " " + r.get<std::string>() + ",";
                }
                requirements.pop_back();
            }
            if (module_value.at("required")) {
                deepseaListItem = new UnTogglableListItem(module_value.at("displayName"), 1, requirements, "Required", "o");
            }
            else {
                deepseaListItem = new ::brls::ToggleListItem(module_value.at("displayName"),
                                                             old_modules.find(module.key()) != old_modules.end() ? 1 : 0,
                                                             requirements,
                                                             Lang::t("common.selected"),
                                                             Lang::t("common.off"));
            }
            name_map.insert(std::pair(module_value.at("displayName"), module.key()));
            deepseaListItem->registerAction(Lang::t("ams_update.show_module_description"), brls::Key::Y, [module_value] {
                util::showDialogBoxInfo(fmt::format("{}:\n{}", module_value.at("repo"), module_value.at("description")));
                return true;
            });
            list->addView(deepseaListItem);
        }
        lists.push_back(list);
        appView->addTab(category.key(), list);
    }

    appView->registerAction(Lang::t("ams_update.download_deepsea_package"), brls::Key::X, [this, lists, name_map] {
        std::set<std::string> desired_modules;
        for (const auto& list : lists) {
            for (size_t i = 0; i < list->getViewsCount(); i++) {
                if (brls::ToggleListItem* item = dynamic_cast<brls::ToggleListItem*>(list->getChild(i))) {
                    if (item->getToggleState()) {
                        desired_modules.insert(name_map.at(item->getLabel()));
                    }
                }
            }
        }

        std::string request_url = DEEPSEA_BUILD_URL;
        for (const auto& e : desired_modules)
            request_url += e + ";";

        this->CreateStagedFrames(Lang::t("common.download") + "Custom DeepSea package" + Lang::t("common.from") + request_url,
                                 request_url,
                                 this->erista);
        return true;
    });
    appView->registerAction("", brls::Key::PLUS, [this] { return true; });

    brls::PopupFrame::open(Lang::t("ams_update.deepsea_builder"), appView, modules.empty() ? Lang::t("ams_update.cant_fetch_deepsea") : Lang::t("ams_update.build_your_deepsea"), "");
}

AmsTab_Custom::AmsTab_Custom(const nlohmann::ordered_json& nxlinks, const bool erista) : AmsTab(nxlinks, erista)
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

bool UnTogglableListItem::onClick()
{
    return true;
}
