#include "JC_page.hpp"

#include "color_swapper.hpp"
#include "confirm_page.hpp"
#include "constants.hpp"
#include "worker_page.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;
JCPage::JCPage() : AppletFrame(true, true)
{
    this->setTitle(Lang::t("joy_con.title"));
    list = new brls::List();
    label = new brls::Label(brls::LabelStyle::DESCRIPTION, fmt::format(Lang::t("joy_con.description"), JC_COLOR_PATH, COLOR_PICKER_URL), true);
    list->addView(label);

    backup = new brls::ListItem(Lang::t("joy_con.backup"));
    backup->getClickEvent()->subscribe([](brls::View* view) {
        brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
        stagedFrame->setTitle(Lang::t("joy_con.label"));
        stagedFrame->addStage(
            new WorkerPage(stagedFrame, Lang::t("joy_con.backing_up"),
                           []() { JC::backupJCColor(JC_COLOR_PATH); }));
        stagedFrame->addStage(
            new ConfirmPage_Done(stagedFrame, Lang::t("common.all_done")));
        brls::Application::pushView(stagedFrame);
    });
    list->addView(backup);

    list->addView(new brls::ListItemGroupSpacing(true));

    auto profiles = JC::getProfiles(JC_COLOR_PATH);
    for (const auto& profile : profiles) {
        std::vector<int> value = profile.second;
        listItem = new brls::ListItem(profile.first);
        listItem->getClickEvent()->subscribe([value](brls::View* view) {
            brls::StagedAppletFrame* stagedFrame = new brls::StagedAppletFrame();
            stagedFrame->setTitle(Lang::t("joy_con.label"));
            stagedFrame->addStage(
                new WorkerPage(stagedFrame, Lang::t("joy_con.changing"),
                               [value]() { JC::changeJCColor(value); }));
            stagedFrame->addStage(
                new ConfirmPage_Done(stagedFrame, Lang::t("joy_con.all_done")));
            brls::Application::pushView(stagedFrame);
        });
        list->addView(listItem);
    }
    this->setContentView(list);
}