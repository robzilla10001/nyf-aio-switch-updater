#include "about_tab.hpp"
#include "lang.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

AboutTab::AboutTab()
{
    // Subtitle
    brls::Label* subTitle = new brls::Label(
        brls::LabelStyle::REGULAR,
        Lang::t("about.title"),
        true);
    subTitle->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(subTitle);

    // Copyright
    brls::Label* copyright = new brls::Label(
        brls::LabelStyle::DESCRIPTION,
        Lang::t("about.copyright") + "\n© 2020-2022 HamletDuFromage",
        true);
    copyright->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->addView(copyright);

    // Links
    this->addView(new brls::Header(Lang::t("about.disclaimers_title")));
    brls::Label* links = new brls::Label(
        brls::LabelStyle::SMALL,
        Lang::t("about.disclaimers") + "\n" + Lang::t("about.donate"),
        true);
    this->addView(links);
}