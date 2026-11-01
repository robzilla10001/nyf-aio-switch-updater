#include "tools_tab.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include "JC_page.hpp"
#include "PC_page.hpp"
#include "app_page.hpp"
#include "cheats_page.hpp"
#include "confirm_page.hpp"
#include "extract.hpp"
#include "fs.hpp"
#include "hide_tabs_page.hpp"
#include "language_page.hpp"
#include "net_page.hpp"
#include "payload_page.hpp"
#include "utils.hpp"
#include "version_compare.hpp"
#include "worker_page.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;
using json = nlohmann::ordered_json;

namespace {
    constexpr const char AppVersion[] = APP_VERSION;
}

void pushAppUpdateFrame(const std::string& tag)
{
    std::string url = fmt::format(APP_URL_FORMAT, tag);
    std::string text(Lang::t("tools.dl_app") + url);
    brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
    stagedFrame->setTitle(Lang::t("common.updating"));
    stagedFrame->addStage(
        new ConfirmPage(stagedFrame, text));
    stagedFrame->addStage(
        new WorkerPage(stagedFrame, Lang::t("common.downloading"), [url]() { util::downloadArchive(url, contentType::app); }));
    stagedFrame->addStage(
        new WorkerPage(stagedFrame, Lang::t("common.extracting"), []() { util::extractArchive(contentType::app); }));
    stagedFrame->addStage(
        new ConfirmPage_AppUpdate(stagedFrame, Lang::t("common.all_done")));
    brls::Application::pushView(stagedFrame);
}

void promptAppUpdate(const std::string& tag)
{
    // Pushing a view while MainFrame is still being pushed would put the dialog underneath it,
    // so wait for the main view to be up. Dialogs are already opened from worker threads elsewhere.
    std::thread([tag]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        brls::Dialog* dialog = new brls::Dialog(fmt::format(Lang::t("main.update_prompt"), tag, AppVersion));
        dialog->addButton(Lang::t("common.yes"), [dialog, tag](brls::View* view) {
            dialog->close([tag]() { pushAppUpdateFrame(tag); });
        });
        dialog->addButton(Lang::t("common.no"), [dialog](brls::View* view) {
            dialog->close();
        });
        dialog->setCancelable(true);
        dialog->open();
    }).detach();
}

ToolsTab::ToolsTab(const std::string& tag, const nlohmann::ordered_json& payloads, bool erista, const nlohmann::ordered_json& hideStatus) : brls::List()
{
    if (util::isNewerVersion(tag, AppVersion)) {
        brls::ListItem* updateApp = new brls::ListItem(fmt::format(Lang::t("tools.update_app"), tag));
        updateApp->getClickEvent()->subscribe([tag](brls::View* view) {
            pushAppUpdateFrame(tag);
        });
        updateApp->setHeight(LISTITEM_HEIGHT);
        this->addView(updateApp);
    }

    brls::ListItem* cheats = new brls::ListItem(Lang::t("tools.cheats"));
    cheats->getClickEvent()->subscribe([](brls::View* view) {
        brls::PopupFrame::open(Lang::t("cheats.menu"), new CheatsPage(), "", "");
    });
    cheats->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* outdatedTitles = new brls::ListItem(Lang::t("tools.outdated_titles"));
    outdatedTitles->getClickEvent()->subscribe([](brls::View* view) {
        brls::PopupFrame::open(Lang::t("tools.outdated_titles"), new AppPage_OutdatedTitles(), Lang::t("tools.outdated_titles_desc"), "");
    });
    outdatedTitles->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* JCcolor = new brls::ListItem(Lang::t("tools.joy_cons"));
    JCcolor->getClickEvent()->subscribe([](brls::View* view) {
        brls::Application::pushView(new JCPage());
    });
    JCcolor->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* PCcolor = new brls::ListItem(Lang::t("tools.pro_cons"));
    PCcolor->getClickEvent()->subscribe([](brls::View* view) {
        brls::Application::pushView(new PCPage());
    });
    PCcolor->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* rebootPayload = new brls::ListItem(Lang::t("tools.inject_payloads"));
    rebootPayload->getClickEvent()->subscribe([](brls::View* view) {
        brls::PopupFrame::open(Lang::t("tools.inject_payloads"), new PayloadPage(), "", "");
    });
    rebootPayload->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* netSettings = new brls::ListItem(Lang::t("tools.internet_settings"));
    netSettings->getClickEvent()->subscribe([](brls::View* view) {
        brls::PopupFrame::open(Lang::t("tools.internet_settings"), new NetPage(), "", "");
    });
    netSettings->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* browser = new brls::ListItem(Lang::t("tools.browser"));
    browser->getClickEvent()->subscribe([](brls::View* view) {
        std::string url;
        if (brls::Swkbd::openForText([&url](std::string text) { url = text; }, "cheatslips.com e-mail", "", 256, "https://duckduckgo.com", 0, "Submit", "https://website.tld")) {
            util::openWebBrowser(url);
        }
    });
    browser->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* move = new brls::ListItem(Lang::t("tools.batch_copy"));
    move->getClickEvent()->subscribe([](brls::View* view) {
        chdir("/");
        std::string error = "";
        if (std::filesystem::exists(COPY_FILES_TXT)) {
            error = fs::copyFiles(COPY_FILES_TXT);
        }
        else {
            error = Lang::t("tools.batch_copy_config_not_found");
        }
        util::showDialogBoxInfo(error);
    });
    move->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* cleanUp = new brls::ListItem(Lang::t("tools.clean_up"));
    cleanUp->getClickEvent()->subscribe([](brls::View* view) {
        std::filesystem::remove(AMS_FILENAME);
        std::filesystem::remove(APP_FILENAME);
        std::filesystem::remove(FIRMWARE_FILENAME);
        std::filesystem::remove(CHEATS_FILENAME);
        std::filesystem::remove(BOOTLOADER_FILENAME);
        std::filesystem::remove(CHEATS_VERSION);
        std::filesystem::remove(CUSTOM_FILENAME);
        fs::removeDir(AMS_DIRECTORY_PATH);
        fs::removeDir(SEPT_DIRECTORY_PATH);
        fs::removeDir(FW_DIRECTORY_PATH);
        util::showDialogBoxInfo(Lang::t("common.all_done"));
    });
    cleanUp->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* languageItem = new brls::ListItem(Lang::t("tools.language"));
    languageItem->getClickEvent()->subscribe([](brls::View* view) {
        brls::AppletFrame* picker = language::createPicker([](const std::string& code) {
            language::save(code);
            util::restartApp();
        });
        brls::PopupFrame::open(Lang::t("tools.language"), picker, "", "");
    });
    languageItem->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* hideTabs = new brls::ListItem(Lang::t("tools.hide_tabs"));
    hideTabs->getClickEvent()->subscribe([](brls::View* view) {
        brls::PopupFrame::open(Lang::t("tools.hide_tabs"), new HideTabsPage(), "", "");
    });
    hideTabs->setHeight(LISTITEM_HEIGHT);

    brls::ListItem* changelog = new brls::ListItem(Lang::t("tools.changelog"));
    changelog->getClickEvent()->subscribe([](brls::View* view) {
        util::openWebBrowser(CHANGELOG_URL);
    });
    changelog->setHeight(LISTITEM_HEIGHT);

    if (!util::getBoolValue(hideStatus, "cheats")) this->addView(cheats);
    if (!util::getBoolValue(hideStatus, "outdatedtitles")) this->addView(outdatedTitles);
    if (!util::getBoolValue(hideStatus, "jccolor")) this->addView(JCcolor);
    if (!util::getBoolValue(hideStatus, "pccolor")) this->addView(PCcolor);
    if (erista && !util::getBoolValue(hideStatus, "rebootpayload")) this->addView(rebootPayload);
    if (!util::getBoolValue(hideStatus, "netsettings")) this->addView(netSettings);
    if (!util::getBoolValue(hideStatus, "browser")) this->addView(browser);
    if (!util::getBoolValue(hideStatus, "move")) this->addView(move);
    if (!util::getBoolValue(hideStatus, "cleanup")) this->addView(cleanUp);
    if (!util::getBoolValue(hideStatus, "language")) this->addView(languageItem);
    this->addView(hideTabs);
    this->addView(changelog);
}
