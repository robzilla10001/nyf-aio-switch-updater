#include "cheats_page.hpp"

#include "app_page.hpp"
#include "confirm_page.hpp"
#include "constants.hpp"
#include "download.hpp"
#include "exclude_page.hpp"
#include "extract.hpp"
#include "utils.hpp"
#include "worker_page.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;
CheatsPage::CheatsPage() : AppletFrame(true, true)
{
    this->setTitle(Lang::t("cheats.menu"));
    list = new brls::List();

    item = new brls::ListItem(Lang::t("cheats.view"));
    item->getClickEvent()->subscribe([](brls::View* view) {
        brls::Application::pushView(new AppPage_DownloadedCheats());
    });
    list->addView(item);

    item = new brls::ListItem(Lang::t("cheats.exclude"));
    item->getClickEvent()->subscribe([](brls::View* view) {
        brls::Application::pushView(new AppPage_Exclude());
    });
    list->addView(item);

    item = new brls::ListItem(Lang::t("cheats.delete_existing"));
    item->getClickEvent()->subscribe([](brls::View* view) {
        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle(Lang::t("cheats.delete_existing"));
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, Lang::t("cheats.deleting"), []() { extract::removeCheats(); }));
        stagedFrame->addStage(
            new ConfirmPage_Done(stagedFrame, Lang::t("common.all_done")));
        brls::Application::pushView(stagedFrame);
    });
    list->addView(item);

    item = new brls::ListItem(Lang::t("cheats.delete_orphaned"));
    item->getClickEvent()->subscribe([](brls::View* view) {
        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle(Lang::t("cheats.delete_orphaned"));
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, Lang::t("cheats.deleting"), []() { extract::removeOrphanedCheats(); }));
        stagedFrame->addStage(
            new ConfirmPage_Done(stagedFrame, Lang::t("common.all_done")));
        brls::Application::pushView(stagedFrame);
    });
    list->addView(item);

    std::string cheatsVer = util::getCheatsVersion();
    if (cheatsVer != "") {
        item = new brls::ListItem(Lang::t("cheats.dl_all"));
        item->getClickEvent()->subscribe([cheatsVer](brls::View* view) {
            std::string url = CHEATS_URL_CONTENTS;
            std::string text(fmt::format(Lang::t("main.get_cheats"), cheatsVer) + Lang::t("common.from") + url);
            brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
            stagedFrame->setTitle(Lang::t("cheats.dl_all"));
            stagedFrame->addStage(
                new ConfirmPage(stagedFrame, text));
            if (cheatsVer != "offline") {
                stagedFrame->addStage(
                    new WorkerPage(stagedFrame, Lang::t("common.downloading"), [url]() { util::downloadArchive(url, contentType::cheats); }));
            }
            stagedFrame->addStage(
                new WorkerPage(stagedFrame, Lang::t("common.extracting"), [cheatsVer]() { extract::extractAllCheats(CHEATS_FILENAME, cheatsVer); }));
            stagedFrame->addStage(
                new ConfirmPage_Done(stagedFrame, Lang::t("common.all_done")));
            brls::Application::pushView(stagedFrame);
        });
        list->addView(item);
    }
    this->setContentView(list);
}