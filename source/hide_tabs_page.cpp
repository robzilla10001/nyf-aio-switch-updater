#include "hide_tabs_page.hpp"

#include <fstream>
#include <json.hpp>

#include "constants.hpp"
#include "fs.hpp"
#include "utils.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;
using json = nlohmann::ordered_json;

HideTabsPage::HideTabsPage() : AppletFrame(true, true)
{
    this->setTitle(Lang::t("hide.title"));
    list = new brls::List();
    label = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        Lang::t("hide.desc"),
        true);
    list->addView(label);

    json hideStatus = fs::parseJsonFile(HIDE_TABS_JSON);

    about = new brls::ToggleListItem(Lang::t("main.about"), util::getBoolValue(hideStatus, "about"));
    list->addView(about);

    ams = new brls::ToggleListItem(Lang::t("main.update_ams"), util::getBoolValue(hideStatus, "atmosphere"));
    list->addView(ams);

    cfws = new brls::ToggleListItem(Lang::t("main.update_bootloaders"), util::getBoolValue(hideStatus, "cfw"));
    list->addView(cfws);

    fws = new brls::ToggleListItem(Lang::t("main.download_firmware"), util::getBoolValue(hideStatus, "firmwares"));
    list->addView(fws);

    cheats = new brls::ToggleListItem(Lang::t("main.download_cheats"), util::getBoolValue(hideStatus, "cheats"));
    list->addView(cheats);
	
    custom = new brls::ToggleListItem(Lang::t("main.custom_downloads"), util::getBoolValue(hideStatus, "custom"));
    list->addView(custom);

    outdatedTitles = new brls::ToggleListItem(Lang::t("tools.outdated_titles"), util::getBoolValue(hideStatus, "outdatedtitles"));
    list->addView(outdatedTitles);

    jccolor = new brls::ToggleListItem(Lang::t("tools.joy_cons"), util::getBoolValue(hideStatus, "jccolor"));
    list->addView(jccolor);

    pccolor = new brls::ToggleListItem(Lang::t("tools.pro_cons"), util::getBoolValue(hideStatus, "pccolor"));
    list->addView(pccolor);

    downloadpayload = new brls::ToggleListItem(Lang::t("tools.dl_payloads"), util::getBoolValue(hideStatus, "downloadpayload"));
    list->addView(downloadpayload);

    rebootpayload = new brls::ToggleListItem(Lang::t("tools.inject_payloads"), util::getBoolValue(hideStatus, "rebootpayload"));
    list->addView(rebootpayload);

    netsettings = new brls::ToggleListItem(Lang::t("tools.internet_settings"), util::getBoolValue(hideStatus, "netsettings"));
    list->addView(netsettings);

    browser = new brls::ToggleListItem(Lang::t("tools.browser"), util::getBoolValue(hideStatus, "browser"));
    list->addView(browser);

    move = new brls::ToggleListItem(Lang::t("tools.batch_copy"), util::getBoolValue(hideStatus, "move"));
    list->addView(move);

    cleanup = new brls::ToggleListItem(Lang::t("tools.clean_up"), util::getBoolValue(hideStatus, "cleanup"));
    list->addView(cleanup);

    language = new brls::ToggleListItem(Lang::t("tools.language"), util::getBoolValue(hideStatus, "language"));
    list->addView(language);

    list->registerAction(Lang::t("cheats.exclude_titles_save"), brls::Key::B, [this] {
        json updatedStatus = json::object();
        updatedStatus["about"] = about->getToggleState();
        updatedStatus["atmosphere"] = ams->getToggleState();											 
        updatedStatus["cfw"] = cfws->getToggleState();
        updatedStatus["firmwares"] = fws->getToggleState();
        updatedStatus["cheats"] = cheats->getToggleState();
        updatedStatus["custom"] = custom->getToggleState();	
        updatedStatus["outdatedtitles"] = outdatedTitles->getToggleState();	
        updatedStatus["jccolor"] = jccolor->getToggleState();
        updatedStatus["pccolor"] = pccolor->getToggleState();
        updatedStatus["downloadpayload"] = downloadpayload->getToggleState();
        updatedStatus["rebootpayload"] = rebootpayload->getToggleState();
        updatedStatus["netsettings"] = netsettings->getToggleState();
        updatedStatus["browser"] = browser->getToggleState();
        updatedStatus["move"] = move->getToggleState();
        updatedStatus["cleanup"] = cleanup->getToggleState();
        updatedStatus["language"] = language->getToggleState();

        fs::writeJsonToFile(updatedStatus, HIDE_TABS_JSON);
        brls::Application::popView();
        return true;
    });
    this->setContentView(list);
}
