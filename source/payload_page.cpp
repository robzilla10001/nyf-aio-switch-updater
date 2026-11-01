#include "payload_page.hpp"

#include "fs.hpp"
#include "utils.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

PayloadPage::PayloadPage() : AppletFrame(true, true)
{
    this->updateActionHint(brls::Key::B, "");
    this->updateActionHint(brls::Key::PLUS, "");

    this->list = new brls::List();
    this->label = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        Lang::t("payloads.select"),
        true);
    this->list->addView(this->label);
    std::vector<std::string> payloads = util::fetchPayloads();
    for (const auto& payload : payloads) {
        std::string payload_path = payload;
        this->listItem = new brls::ListItem(payload_path);
        this->listItem->getClickEvent()->subscribe([payload_path](brls::View* view) {
            util::rebootToPayload(payload_path);
            brls::Application::popView();
        });
        this->RegisterCopyAction(brls::Key::X, payload_path, REBOOT_PAYLOAD_PATH, Lang::t("payloads.set_reboot_payload"));
        this->RegisterCopyAction(brls::Key::Y, payload_path, UPDATE_BIN_PATH, Lang::t("payloads.set_update_bin"));
        this->list->addView(this->listItem);
    }
    this->list->addView(new brls::ListItemGroupSpacing(true));

    this->listItem = new brls::ListItem(Lang::t("common.shut_down"));
    this->listItem->getClickEvent()->subscribe([](brls::View* view) {
        util::shutDown(false);
        brls::Application::popView();
    });
    this->list->addView(this->listItem);

    this->listItem = new brls::ListItem(Lang::t("common.reboot"));
    this->listItem->getClickEvent()->subscribe([](brls::View* view) {
        util::shutDown(true);
        brls::Application::popView();
    });
    list->addView(this->listItem);

    this->setContentView(list);
}

void PayloadPage::RegisterCopyAction(brls::Key key, const std::string& payload_path, const std::string& payload_dest, const std::string& action_name)
{
    this->listItem->registerAction(action_name, key, [payload_path, payload_dest] {
        std::string res = fs::copyFile(payload_path, payload_dest)
                              ? fmt::format(Lang::t("payloads.copy_success"), payload_path, payload_dest)
                              : "Failed.";
        util::showDialogBoxInfo(res);
        return true;
    });
}