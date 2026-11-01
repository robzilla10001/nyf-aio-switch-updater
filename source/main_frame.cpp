#include "main_frame.hpp"

#include <fstream>
#include <json.hpp>

#include "about_tab.hpp"
#include "ams_tab.hpp"
#include "download.hpp"
#include "fs.hpp"
#include "list_download_tab.hpp"
#include "tools_tab.hpp"
#include "utils.hpp"
#include "version_compare.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;
using json = nlohmann::ordered_json;

namespace {
    constexpr const char AppTitle[] = APP_TITLE;
    constexpr const char AppVersion[] = APP_VERSION;
}  // namespace

MainFrame::MainFrame() : TabFrame()
{
    this->setIcon("romfs:/gui_icon.png");
    this->setTitle(AppTitle);

    s64 freeStorage;
    std::string tag = util::getLatestTag(APP_REPO);
    bool updateAvailable = util::isNewerVersion(tag, AppVersion);
    this->setFooterText(fmt::format(Lang::t("main.footer_text"),
                                    updateAvailable ? AppVersion + Lang::t("main.new_update") : AppVersion,
                                    R_SUCCEEDED(fs::getFreeStorageSD(freeStorage)) ? (float)freeStorage / 0x40000000 : -1));

    json hideStatus = fs::parseJsonFile(HIDE_TABS_JSON);
    nlohmann::ordered_json nxlinks;
    download::getRequest(NXLINKS_URL, nxlinks);

    bool erista = util::isErista();

    if (!util::getBoolValue(hideStatus, "about"))
        this->addTab(Lang::t("main.about"), new AboutTab());

    if (!util::getBoolValue(hideStatus, "atmosphere"))
        this->addTab(Lang::t("main.update_ams"), new AmsTab_Regular(erista));

    if (!util::getBoolValue(hideStatus, "cfw"))
        this->addTab(Lang::t("main.update_bootloaders"), new ListDownloadTab(contentType::bootloaders, nxlinks));

    if (!util::getBoolValue(hideStatus, "firmwares"))
        this->addTab(Lang::t("main.download_firmware"), new ListDownloadTab(contentType::fw, nxlinks));

    if (!util::getBoolValue(hideStatus, "cheats"))
        this->addTab(Lang::t("main.download_cheats"), new ListDownloadTab(contentType::cheats));

    if (!util::getBoolValue(hideStatus, "custom"))
        this->addTab(Lang::t("main.custom_downloads"), new AmsTab_Custom(erista));

    if (!util::getBoolValue(hideStatus, "tools"))
        this->addTab(Lang::t("main.tools"), new ToolsTab(tag, util::getValueFromKey(nxlinks, "payloads"), erista, hideStatus));

    this->registerAction("", brls::Key::B, [this] { return true; });

    if (updateAvailable)
        promptAppUpdate(tag);
}
