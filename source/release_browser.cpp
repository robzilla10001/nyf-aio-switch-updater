#include "release_browser.hpp"

#include <borealis.hpp>

#include "constants.hpp"
#include "lang.hpp"
#include "utils.hpp"

namespace release_browser {

    namespace {

        // A paragraph of release notes. Being focusable is what lets the list scroll through long notes.
        class NotesParagraph : public brls::Label
        {
        public:
            NotesParagraph(const std::string& text) : brls::Label(brls::LabelStyle::SMALL, text, true) {}
            brls::View* getDefaultFocus() override { return this; }
        };

        bool hasExtension(const std::string& name, const std::vector<std::string>& extensions)
        {
            if (extensions.empty())
                return true;
            for (const auto& ext : extensions) {
                if (github::detail::endsWith(github::detail::lower(name), "." + github::detail::lower(ext)))
                    return true;
            }
            return false;
        }

        brls::List* createReleaseView(const github::Release& release, const std::vector<std::string>& extensions, const AssetHandler& onAsset)
        {
            brls::List* list = new brls::List();

            std::string heading = release.name.empty() ? release.tag : release.name;
            list->addView(new brls::Label(brls::LabelStyle::DESCRIPTION,
                                          fmt::format(Lang::t("github.release_info"), heading, github::shortDate(release.published),
                                                      release.prerelease ? Lang::t("github.prerelease") : ""),
                                          true));

            list->addView(new brls::Header(Lang::t("github.files")));
            bool any = false;
            for (const auto& asset : release.assets) {
                if (!hasExtension(asset.name, extensions))
                    continue;
                any = true;
                brls::ListItem* item = new brls::ListItem(asset.name, "", fmt::format("{:.1f} MB", asset.size / 1048576.0));
                github::Release releaseCopy = release;
                item->getClickEvent()->subscribe([onAsset, releaseCopy, asset](brls::View* view) {
                    onAsset(releaseCopy, asset);
                });
                list->addView(item);
            }
            if (!any)
                list->addView(new brls::Label(brls::LabelStyle::DESCRIPTION, Lang::t("github.no_files"), true));

            std::vector<std::string> notes = github::splitParagraphs(github::plainText(release.body));
            if (!notes.empty()) {
                list->addView(new brls::Header(Lang::t("github.release_notes")));
                for (const auto& paragraph : notes)
                    list->addView(new NotesParagraph(paragraph));
            }
            return list;
        }

    }  // namespace

    std::string statusText(github::Status status)
    {
        switch (status) {
            case github::Status::NoNetwork:
                return Lang::t("github.no_network");
            case github::Status::RateLimited:
                return Lang::t("github.rate_limited");
            case github::Status::NotFound:
                return Lang::t("github.no_releases");
            default:
                return Lang::t("github.bad_response");
        }
    }

    void open(const std::string& title, const std::string& repo, int count, const std::vector<std::string>& extensions,
              const AssetHandler& onAsset, const std::string& icon, const std::string& subtitle)
    {
        std::vector<github::Release> releases;
        github::Status status = github::getReleases(repo, releases, count);
        if (status == github::Status::Ok && releases.empty())
            status = github::Status::NotFound;
        if (status != github::Status::Ok) {
            util::showDialogBoxInfo(statusText(status));
            return;
        }

        brls::TabFrame* frame = new brls::TabFrame();
        if (!icon.empty())
            frame->setIcon(icon);
        for (const auto& release : releases)
            frame->addTab(release.tag, createReleaseView(release, extensions, onAsset));
        frame->registerAction("", brls::Key::PLUS, [] { return true; });

        brls::PopupFrame::open(title, frame, subtitle, "");
    }

}  // namespace release_browser
