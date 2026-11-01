#include <switch.h>

#include <borealis.hpp>
#include <filesystem>
#include <json.hpp>

#include "constants.hpp"
#include "fs.hpp"
#include "main_frame.hpp"
#include "lang.hpp"
#include "language_page.hpp"
#include "warning_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

//TimeServiceType __nx_time_service_type = TimeServiceType_System;

namespace {

    // Disclaimer on the very first launch, then the main view (About is its first tab)
    void showMain()
    {
        if (std::filesystem::exists(HIDDEN_AIO_FILE)) {
            brls::Application::pushView(new MainFrame());
        }
        else {
            brls::Application::pushView(new WarningPage(Lang::t("main.launch_warning")));
        }
    }

}  // namespace

int main(int argc, char* argv[])
{
    // Init the app
    if (!brls::Application::init(APP_TITLE)) {
        brls::Logger::error("Unable to init Borealis application");
        return EXIT_FAILURE;
    }

    // Borealis' own strings (button hints) are English only; the app's text goes through Lang
    i18n::loadTranslations("en-US");

        //appletInitializeGamePlayRecording();

        // Setup verbose logging on PC
#ifndef __SWITCH__
    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
#endif

    setsysInitialize();
    plInitialize(PlServiceType_User);
    nsInitialize();
    socketInitializeDefault();
    nxlinkStdio();
    pmdmntInitialize();
    pminfoInitialize();
    splInitialize();
    romfsInit();

    fs::createTree(CONFIG_PATH);
    fs::createTree(std::string(LANG_PATH) + "/");
    Lang::init(LANG_PATH, LANG_EMBEDDED);

    brls::Logger::setLogLevel(brls::LogLevel::DEBUG);
    brls::Logger::debug("Start");

    if (language::loadSaved()) {
        showMain();
    }
    else {
        // first run (or the settings file is gone): let the user pick a language first
        brls::AppletFrame* picker = language::createPicker([](const std::string& code) {
            language::save(code);
            Lang::load(code);
            showMain();
        });
        picker->registerAction("", brls::Key::B, [] { return true; });
        brls::Application::pushView(picker);
    }

    while (brls::Application::mainLoop())
        ;

    romfsExit();
    splExit();
    pminfoExit();
    pmdmntExit();
    socketExit();
    nsExit();
    setsysExit();
    plExit();
    return EXIT_SUCCESS;
}
