#include "language_page.hpp"

#include <filesystem>
#include <fstream>
#include <json.hpp>

#include "constants.hpp"
#include "fs.hpp"
#include "lang.hpp"

namespace language {

    bool loadSaved()
    {
        nlohmann::ordered_json settings = fs::parseJsonFile(LANGUAGE_JSON);
        if (!settings.is_object() || !settings.contains("language") || !settings["language"].is_string())
            return false;
        Lang::load(settings["language"].get<std::string>());
        return true;
    }

    void save(const std::string& code)
    {
        fs::createTree(CONFIG_PATH);
        nlohmann::ordered_json settings = nlohmann::ordered_json::object();
        settings["language"] = code;
        std::ofstream out(LANGUAGE_JSON);
        out << settings.dump();
    }

    brls::AppletFrame* createPicker(const std::function<void(const std::string& code)>& onSelected)
    {
        brls::AppletFrame* frame = new brls::AppletFrame(true, true);
        brls::List* list = new brls::List();

        for (const auto& info : Lang::scan()) {
            brls::ListItem* item = new brls::ListItem(info.name + " (" + info.code + ")");
            item->setHeight(LISTITEM_HEIGHT);
            std::string code = info.code;
            item->getClickEvent()->subscribe([onSelected, code](brls::View* view) {
                onSelected(code);
            });
            list->addView(item);
        }

        frame->setTitle(Lang::t("tools.language"));
        frame->setContentView(list);
        return frame;
    }

}  // namespace language
