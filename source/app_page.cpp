#include "app_page.hpp"

#include <switch.h>

#include <filesystem>
#include <fstream>
#include <regex>

#include "confirm_page.hpp"
#include "download.hpp"
#include "download_cheats_page.hpp"
#include "extract.hpp"
#include "fs.hpp"
#include "utils.hpp"
#include "worker_page.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

AppPage::AppPage() : AppletFrame(true, true)
{
    list = new brls::List();
}

void AppPage::PopulatePage()
{
    this->CreateLabel();

    NsApplicationRecord* records = new NsApplicationRecord[MaxTitleCount];
    NsApplicationControlData* controlData = NULL;
    std::string name;

    s32 recordCount = 0;
    u64 controlSize = 0;
    u64 tid;

    if (!util::isApplet()) {
        if (R_SUCCEEDED(nsListApplicationRecord(records, MaxTitleCount, 0, &recordCount))) {
            for (s32 i = 0; i < recordCount; i++) {
                controlSize = 0;

                if (R_FAILED(InitControlData(&controlData))) break;

                tid = records[i].application_id;

                if (R_FAILED(GetControlData(tid, controlData, controlSize, name)))
                    continue;

                listItem = new brls::ListItem(name, "", util::formatApplicationId(tid));
                listItem->setThumbnail(controlData->icon, sizeof(controlData->icon));
                this->AddListItem(name, tid);
            }
            free(controlData);
        }
    }
    else {
        tid = GetCurrentApplicationId();
        if (R_SUCCEEDED(InitControlData(&controlData)) && R_SUCCEEDED(GetControlData(tid & 0xFFFFFFFFFFFFF000, controlData, controlSize, name))) {
            listItem = new brls::ListItem(name, "", util::formatApplicationId(tid));
            listItem->setThumbnail(controlData->icon, sizeof(controlData->icon));
            this->AddListItem(name, tid);
        }
        label = new brls::Label(brls::LabelStyle::SMALL, Lang::t("common.applet_mode_not_supported"), true);
        list->addView(label);
    }
    delete[] records;

    brls::Logger::debug("count {}", list->getViewsCount());

    if (!list->getViewsCount()) {
        list->addView(new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("common.nothing_to_see"), true));
    }

    this->setContentView(list);
}

void AppPage::CreateDownloadAllButton()
{
    std::string text(Lang::t("cheats.downloading"));
    std::string url = CHEATS_URL_CONTENTS;
    text += url;
    download = new brls::ListItem(Lang::t("cheats.dl_latest"));
    download->getClickEvent()->subscribe([url, text](brls::View* view) {
        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle(Lang::t("cheats.getting_cheats"));
        stagedFrame->addStage(
            new ConfirmPage(stagedFrame, text));
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, Lang::t("common.downloading"), [url]() { util::downloadArchive(url, contentType::cheats); }));
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, Lang::t("common.extracting"), []() { util::extractArchive(contentType::cheats); }));
        stagedFrame->addStage(
            new ConfirmPage_Done(stagedFrame, Lang::t("common.all_done")));
        brls::Application::pushView(stagedFrame);
    });
    list->addView(download);
}

u32 AppPage::InitControlData(NsApplicationControlData** controlData)
{
    free(*controlData);
    *controlData = (NsApplicationControlData*)malloc(sizeof(NsApplicationControlData));
    if (*controlData == NULL) {
        return 300;
    }
    else {
        memset(*controlData, 0, sizeof(NsApplicationControlData));
        return 0;
    }
}

u32 AppPage::GetControlData(u64 tid, NsApplicationControlData* controlData, u64& controlSize, std::string& name)
{
    Result rc;
    NacpLanguageEntry* langEntry = NULL;
    rc = nsGetApplicationControlData(NsApplicationControlSource_Storage, tid, controlData, sizeof(NsApplicationControlData), &controlSize);
    if (R_FAILED(rc)) return rc;

    if (controlSize < sizeof(controlData->nacp)) return 100;

    rc = nacpGetLanguageEntry(&controlData->nacp, &langEntry);
    if (R_FAILED(rc)) return rc;

    if (!langEntry->name) return 200;

    name = langEntry->name;

    return 0;
}

void AppPage::AddListItem(const std::string& name, u64 tid)
{
    list->addView(listItem);
}

uint64_t AppPage::GetCurrentApplicationId()
{
    Result rc = 0;
    uint64_t pid = 0;
    uint64_t tid = 0;

    rc = pmdmntGetApplicationProcessId(&pid);
    if (rc == 0x20f || R_FAILED(rc)) return 0;

    rc = pminfoGetProgramId(&tid, pid);
    if (rc == 0x20f || R_FAILED(rc)) return 0;

    return tid;
}

AppPage_CheatSlips::AppPage_CheatSlips() : AppPage()
{
    this->PopulatePage();
    this->registerAction(Lang::t("cheats.cheatslips_logout"), brls::Key::X, [this] {
        std::filesystem::remove(TOKEN_PATH);
        return true;
    });
}

void AppPage_CheatSlips::CreateLabel()
{
    this->setTitle(Lang::t("cheats.cheatslips_title"));
    label = new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("cheats.cheatslips_select"), true);
    list->addView(label);
}

void AppPage_CheatSlips::AddListItem(const std::string& name, u64 tid)
{
    listItem->getClickEvent()->subscribe([tid, name](brls::View* view) { brls::Application::pushView(new DownloadCheatsPage_CheatSlips(tid, name)); });
    list->addView(listItem);
}

AppPage_Gbatemp::AppPage_Gbatemp() : AppPage()
{
    this->PopulatePage();
    this->setIcon("romfs:/gbatemp_icon.png");
}

void AppPage_Gbatemp::CreateLabel()
{
    this->setTitle(Lang::t("cheats.gbatemp_title"));
    label = new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("cheats.cheatslips_select"), true);
    list->addView(label);
}

void AppPage_Gbatemp::AddListItem(const std::string& name, u64 tid)
{
    listItem->getClickEvent()->subscribe([tid, name](brls::View* view) { brls::Application::pushView(new DownloadCheatsPage_Gbatemp(tid, name)); });
    list->addView(listItem);
}

AppPage_Gfx::AppPage_Gfx() : AppPage()
{
    this->PopulatePage();
}

void AppPage_Gfx::CreateLabel()
{
    this->setTitle(Lang::t("cheats.gfx_title"));
    label = new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("cheats.cheatslips_select"), true);
    list->addView(label);
}

void AppPage_Gfx::AddListItem(const std::string& name, u64 tid)
{
    listItem->getClickEvent()->subscribe([tid, name](brls::View* view) { brls::Application::pushView(new DownloadCheatsPage_Gfx(tid, name)); });
    list->addView(listItem);
}


AppPage_Exclude::AppPage_Exclude() : AppPage()
{
    this->PopulatePage();
}

void AppPage_Exclude::CreateLabel()
{
    this->setTitle(Lang::t("cheats.exclude_titles"));
    label = new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("cheats.exclude_titles_desc"), true);
    list->addView(label);
}

void AppPage_Exclude::PopulatePage()
{
    this->CreateLabel();

    NsApplicationRecord* records = new NsApplicationRecord[MaxTitleCount];
    NsApplicationControlData* controlData = NULL;
    std::string name;

    s32 recordCount = 0;
    u64 controlSize = 0;
    u64 tid;

    auto titles = fs::readLineByLine(CHEATS_EXCLUDE);

    if (!util::isApplet()) {
        if (R_SUCCEEDED(nsListApplicationRecord(records, MaxTitleCount, 0, &recordCount))) {
            for (s32 i = 0; i < recordCount; i++) {
                controlSize = 0;

                if (R_FAILED(InitControlData(&controlData))) break;

                tid = records[i].application_id;

                if (R_FAILED(GetControlData(tid, controlData, controlSize, name)))
                    continue;

                brls::ToggleListItem* listItem;
                listItem = new brls::ToggleListItem(std::string(name), titles.find(util::formatApplicationId(tid)) != titles.end() ? 0 : 1);

                listItem->setThumbnail(controlData->icon, sizeof(controlData->icon));
                items.insert(std::make_pair(listItem, util::formatApplicationId(tid)));
                list->addView(listItem);
            }
            free(controlData);
        }
    }
    else {
        label = new brls::Label(brls::LabelStyle::SMALL, Lang::t("common.applet_mode_not_supported"), true);
        list->addView(label);
    }
    delete[] records;

    list->registerAction(Lang::t("cheats.exclude_titles_save"), brls::Key::B, [this] {
        std::set<std::string> exclude;
        for (const auto& item : items) {
            if (!item.first->getToggleState()) {
                exclude.insert(item.second);
            }
        }
        extract::writeTitlesToFile(exclude, CHEATS_EXCLUDE);
        brls::Application::popView();
        return true;
    });

    this->registerAction(Lang::t("cheats.exclude_all"), brls::Key::X, [this] {
        std::set<std::string> exclude;
        for (const auto& item : items) {
            exclude.insert(item.second);
        }
        extract::writeTitlesToFile(exclude, CHEATS_EXCLUDE);
        brls::Application::popView();
        return true;
    });

    this->registerAction(Lang::t("cheats.exclude_none"), brls::Key::Y, [this] {
        extract::writeTitlesToFile({}, CHEATS_EXCLUDE);
        brls::Application::popView();
        return true;
    });

    this->setContentView(list);
}

AppPage_DownloadedCheats::AppPage_DownloadedCheats() : AppPage()
{
    GetExistingCheatsTids();
    this->PopulatePage();
}

void AppPage_DownloadedCheats::CreateLabel()
{
    this->setTitle(Lang::t("cheats.installed"));
    label = new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("cheats.label"), true);
    list->addView(label);
}

void AppPage_DownloadedCheats::AddListItem(const std::string& name, u64 tid)
{
    auto tid_str = util::formatApplicationId(tid);
    if (titles.find(tid_str) != titles.end()) {
        listItem->getClickEvent()->subscribe([tid, name](brls::View* view) { cheats_util::ShowCheatFiles(tid, name); });
        listItem->registerAction(Lang::t("cheats.delete_cheats"), brls::Key::Y, [tid_str] {
            util::showDialogBoxInfo(extract::removeCheatsDirectory(fmt::format("{}{}", util::getContentsPath(), tid_str)) ? Lang::t("common.all_done") : fmt::format(Lang::t("cheats.deletion_error"), tid_str));
            return true;
        });
        list->addView(listItem);
    }
}

void AppPage_DownloadedCheats::GetExistingCheatsTids()
{
    std::string path = util::getContentsPath();
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string cheatsPath = entry.path().string() + "/cheats";
        if (std::filesystem::exists(cheatsPath) && !std::filesystem::is_empty(cheatsPath)) {
            for (const auto& cheatFile : std::filesystem::directory_iterator(cheatsPath)) {
                if (extract::isBID(cheatFile.path().filename().stem())) {
                    titles.insert(util::upperCase(cheatsPath.substr(cheatsPath.length() - 7 - 16, 16)));
                    break;
                }
            }
        }
    }
}

AppPage_OutdatedTitles::AppPage_OutdatedTitles() : AppPage()
{
    download::getRequest(LOOKUP_TABLE_URL, versions);
    if (versions.empty()) {
        list->addView(new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("main.links_not_found"), true));
        this->setContentView(list);
    }
    else {
        this->PopulatePage();
    }
}

void AppPage_OutdatedTitles::AddListItem(const std::string& name, u64 tid)
{
    u32 version = cheats_util::GetVersion(tid);
    std::string tid_string = util::formatApplicationId(tid);
    if (versions.find(tid_string) != versions.end()) {
        u32 latest = versions.at(tid_string).at("latest");
        if (version < latest) {
            listItem->setSubLabel(fmt::format("{}\t|\t v{} (local) → v{} (latest)", tid_string, version, latest));
            list->addView(listItem);
        }
    }
    /* else {
        listItem->setSubLabel(fmt::format("{}\t|\t {}", tid_string, Lang::t("tools.latest_version_not_found")));
        list->addView(listItem);
    } */
}