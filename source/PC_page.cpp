#include "PC_page.hpp"

#include "color_swapper.hpp"
#include "confirm_page.hpp"
#include "constants.hpp"
#include "worker_page.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;
PCPage::PCPage() : AppletFrame(true, true)
{
    this->setTitle(Lang::t("pro_con.title"));
    list = new brls::List();
    std::string labelText = fmt::format("{}\n{}", Lang::t("pro_con.desc"), Lang::t("pro_con.warning"));
    label = new brls::Label(brls::LabelStyle::DESCRIPTION, labelText, true);
    list->addView(label);

    backup = new brls::ListItem(Lang::t("joy_con.backup"));
    backup->getClickEvent()->subscribe([](brls::View* view) {
        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle(Lang::t("pro_con.label"));
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, Lang::t("pro_con.backing_up"),
                           []() { PC::backupPCColor(PC_COLOR_PATH); }));
        stagedFrame->addStage(
            new ConfirmPage_Done(stagedFrame, Lang::t("common.all_done")));
        brls::Application::pushView(stagedFrame);
    });
    list->addView(backup);

    list->addView(new brls::ListItemGroupSpacing(true));

    auto profiles = PC::getProfiles(PC_COLOR_PATH);
    for (const auto& profile : profiles) {
        std::vector<int> value = profile.second;
        listItem = new brls::ListItem(profile.first);
        listItem->getClickEvent()->subscribe([value](brls::View* view) {
            brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
            stagedFrame->setTitle(Lang::t("pro_con.label"));
            stagedFrame->addStage(
                new WorkerPage(stagedFrame, Lang::t("pro_con.changing"),
                               [value]() { PC::changePCColor(value); }));
            stagedFrame->addStage(
                new ConfirmPage_Done(stagedFrame, Lang::t("pro_con.all_done")));
            brls::Application::pushView(stagedFrame);
        });
        list->addView(listItem);
    }
    this->setContentView(list);
}