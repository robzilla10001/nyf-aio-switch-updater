#include "download_payload_page.hpp"

#include "confirm_page.hpp"
#include "download.hpp"
#include "fs.hpp"
#include "utils.hpp"
#include "worker_page.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;
DownloadPayloadPage::DownloadPayloadPage(const nlohmann::ordered_json& payloads) : AppletFrame(true, true)
{
    this->setTitle(Lang::t("payloads.dl_payloads"));
    list = new brls::List();
    label = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        Lang::t("payloads.select") + std::string(BOOTLOADER_PL_PATH) + ".",
        true);
    list->addView(label);

    auto links = download::getLinksFromJson(payloads);
    if (links.size()) {
        for (const auto& link : links) {
            std::string url = link.second;
            std::string path = std::string(BOOTLOADER_PL_PATH) + link.first;
            std::string text(Lang::t("common.download") + link.first + Lang::t("common.from") + url);
            listItem = new brls::ListItem(link.first);
            listItem->getClickEvent()->subscribe([text, url, path](brls::View* view) {
                fs::createTree(BOOTLOADER_PL_PATH);
                brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
                stagedFrame->setTitle(Lang::t("tools.getting_payload"));
                stagedFrame->addStage(
                    new ConfirmPage(stagedFrame, text));
                stagedFrame->addStage(
                    new WorkerPage(stagedFrame, Lang::t("common.downloading"), [url, path]() { download::downloadFile(url, path, OFF); }));
                stagedFrame->addStage(
                    new ConfirmPage_Done(stagedFrame, Lang::t("common.all_done")));
                brls::Application::pushView(stagedFrame);
            });
            list->addView(listItem);
        }
    }
    else {
        notFound = new brls::Label(
            brls::LabelStyle::DESCRIPTION,
            Lang::t("main.links_not_found"),
            true);
        notFound->setHorizontalAlign(NVG_ALIGN_CENTER);
        list->addView(notFound);
        brls::ListItem* back = new brls::ListItem(Lang::t("common.back"));
        back->getClickEvent()->subscribe([](brls::View* view) {
            brls::Application::popView();
        });
        list->addView(back);
    }
    this->setContentView(list);
}