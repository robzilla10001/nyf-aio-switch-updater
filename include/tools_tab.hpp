#pragma once

#include <borealis.hpp>
#include <json.hpp>
//#include "ntcp.hpp"

// Stages: confirm -> download -> extract -> relaunch through the forwarder
void pushAppUpdateFrame(const std::string& tag);
// Asks whether to update to `tag`; returns immediately, the dialog is shown once the main view is up
void promptAppUpdate(const std::string& tag);

class ToolsTab : public brls::List
{
private:
    brls::StagedAppletFrame* stagedFrame;

public:
    ToolsTab(const std::string& tag, const nlohmann::ordered_json& payloads, bool erista = true, const nlohmann::ordered_json& hideStatus = {});
};