#pragma once

constexpr const char ROOT_PATH[] = "/";
constexpr const char APP_PATH[] = "/switch/nyf-aio-switch-updater/";
constexpr const char NRO_PATH[] = "/switch/nyf-aio-switch-updater/nyf-aio-switch-updater.nro";
constexpr const char NRO_PATH_REGEX[] = ".*(/switch/.*nyf-aio-switch-updater.nro).*";
constexpr const char DOWNLOAD_PATH[] = "/config/nyf-aio-switch-updater/";
constexpr const char CONFIG_PATH[] = "/config/nyf-aio-switch-updater/";
constexpr const char CONFIG_FILE[] = "/config/nyf-aio-switch-updater/config.json";
constexpr const char CONFIG_PATH_UNZIP[] = "config\\nyf-aio-switch-updater";

constexpr const char RCM_PAYLOAD_PATH[] = "romfs:/aio_rcm.bin";
constexpr const char MARIKO_PAYLOAD_PATH[] = "/payload.bin";
constexpr const char MARIKO_PAYLOAD_PATH_TEMP[] = "/payload.bin.aio";

constexpr const char CHANGELOG_URL[] = "https://github.com/robzilla10001/nyf-aio-switch-updater/releases";

// {} is the release tag
constexpr const char APP_URL_FORMAT[] = "https://github.com/robzilla10001/nyf-aio-switch-updater/releases/download/{}/nyf-aio-switch-updater.zip";
constexpr const char APP_REPO[] = "robzilla10001/nyf-aio-switch-updater";
constexpr const char AMS_REPO[] = "Atmosphere-NX/Atmosphere";
constexpr const char BTH_REPO[] = "borntohonk/Atmosphere";
constexpr const char HEKATE_REPO[] = "CTCaer/hekate";
constexpr const char APP_FILENAME[] = "/config/nyf-aio-switch-updater/app.zip";

constexpr const char NXLINKS_URL[] = "https://raw.githubusercontent.com/HamletDuFromage/nx-links/master/nx-links.json";

constexpr const char CUSTOM_FILENAME[] = "/config/nyf-aio-switch-updater/custom.zip";
constexpr const char HEKATE_IPL_PATH[] = "/bootloader/hekate_ipl.ini";

constexpr const char FIRMWARE_URL[] = "https://raw.githubusercontent.com/HamletDuFromage/nx-links/master/firmwares.json";
constexpr const char FIRMWARE_FILENAME[] = "/config/nyf-aio-switch-updater/firmware.zip";
constexpr const char FIRMWARE_PATH[] = "/firmware/";

constexpr const char CFW_URL[] = "https://raw.githubusercontent.com/HamletDuFromage/nx-links/master/bootloaders.json";
constexpr const char BOOTLOADER_FILENAME[] = "/config/nyf-aio-switch-updater/bootloader.zip";

constexpr const char AMS_URL[] = "https://raw.githubusercontent.com/HamletDuFromage/nx-links/master/cfws.json";
constexpr const char AMS_FILENAME[] = "/config/nyf-aio-switch-updater/ams.zip";

constexpr const char HEKATE_URL[] = "https://raw.githubusercontent.com/HamletDuFromage/nx-links/master/hekate.json";

constexpr const char PAYLOAD_URL[] = "https://raw.githubusercontent.com/HamletDuFromage/nx-links/master/payloads.json";


constexpr const char CUSTOM_PACKS_PATH[] = "/config/nyf-aio-switch-updater/custom_packs.json";

constexpr const char CHEATS_URL_CONTENTS[] = "https://github.com/HamletDuFromage/switch-cheats-db/releases/latest/download/contents.zip";
constexpr const char GFX_CHEATS_URL_CONTENTS[] = "https://github.com/HamletDuFromage/switch-cheats-db/releases/latest/download/contents_60fps-res-gfx.zip";
constexpr const char CHEATS_URL_VERSION[] = "https://github.com/HamletDuFromage/switch-cheats-db/releases/latest/download/VERSION";
constexpr const char LOOKUP_TABLE_URL[] = "https://raw.githubusercontent.com/HamletDuFromage/switch-cheats-db/master/versions.json";
constexpr const char LOOKUP_TABLE_CBOR[] = "https://github.com/HamletDuFromage/switch-cheats-db/raw/master/versions.cbor";
constexpr const char VERSIONS_DIRECTORY[] = "https://raw.githubusercontent.com/HamletDuFromage/switch-cheats-db/master/versions/";
constexpr const char CHEATS_DIRECTORY[] = "https://raw.githubusercontent.com/HamletDuFromage/switch-cheats-db/master/cheats/";
constexpr const char CHEATS_DIRECTORY_GBATEMP[] = "https://raw.githubusercontent.com/HamletDuFromage/switch-cheats-db/master/cheats_gbatemp/";
constexpr const char CHEATS_DIRECTORY_GFX[] = "https://raw.githubusercontent.com/HamletDuFromage/switch-cheats-db/master/cheats_gfx/";
constexpr const char CHEATSLIPS_CHEATS_URL[] = "https://www.cheatslips.com/api/v1/cheats/";
constexpr const char CHEATSLIPS_TOKEN_URL[] = "https://www.cheatslips.com/api/v1/token";
constexpr const char TOKEN_PATH[] = "/config/nyf-aio-switch-updater/token.json";
constexpr const char CHEATS_FILENAME[] = "/config/nyf-aio-switch-updater/cheats.zip";
constexpr const char CHEATS_EXCLUDE[] = "/config/nyf-aio-switch-updater/exclude.txt";
constexpr const char FILES_IGNORE[] = "/config/nyf-aio-switch-updater/preserve.txt";
constexpr const char INTERNET_JSON[] = "/config/nyf-aio-switch-updater/internet.json";
constexpr const char UPDATED_TITLES_PATH[] = "/config/nyf-aio-switch-updater/updated.dat";
constexpr const char CHEATS_VERSION[] = "/config/nyf-aio-switch-updater/cheats_version.dat";
constexpr const char AMS_CONTENTS[] = "/atmosphere/contents/";
constexpr const char AMS_PATH[] = "/atmosphere/";
constexpr const char CONTENTS_PATH[] = "contents/";

constexpr const char COLOR_PICKER_URL[] = "https://git.io/jcpicker";
constexpr const char JC_COLOR_URL[] = "https://raw.githubusercontent.com/HamletDuFromage/aio-switch-updater/master/jc_profiles.json";
constexpr const char JC_COLOR_PATH[] = "/config/nyf-aio-switch-updater/jc_profiles.json";
constexpr const char PC_COLOR_URL[] = "https://raw.githubusercontent.com/HamletDuFromage/aio-switch-updater/master/pc_profiles.json";
constexpr const char PC_COLOR_PATH[] = "/config/nyf-aio-switch-updater/pc_profiles.json";

constexpr const char PAYLOAD_PATH[] = "/payloads/";
constexpr const char BOOTLOADER_PATH[] = "/bootloader/";
constexpr const char BOOTLOADER_PL_PATH[] = "/bootloader/payloads/";
constexpr const char UPDATE_BIN_PATH[] = "/bootloader/update.bin";
constexpr const char REBOOT_PAYLOAD_PATH[] = "/atmosphere/reboot_payload.bin";
constexpr const char FUSEE_SECONDARY[] = "/atmosphere/fusee-secondary.bin";
constexpr const char FUSEE_MTC[] = "/atmosphere/fusee-mtc.bin";

constexpr const char AMS_DIRECTORY_PATH[] = "/config/nyf-aio-switch-updater/atmosphere/";
constexpr const char SEPT_DIRECTORY_PATH[] = "/config/nyf-aio-switch-updater/sept/";
constexpr const char FW_DIRECTORY_PATH[] = "/firmware/";

constexpr const char HIDE_TABS_JSON[] = "/config/nyf-aio-switch-updater/hide_tabs.json";
constexpr const char COPY_FILES_TXT[] = "/config/nyf-aio-switch-updater/copy_files.txt";
constexpr const char LANGUAGE_JSON[] = "/config/nyf-aio-switch-updater/language.json";
constexpr const char HOMEBREW[] = "/config/nyf-aio-switch-updater/language.json";

constexpr const char ROMFS_PATH[] = "romfs:/";
constexpr const char ROMFS_FORWARDER[] = "romfs:/aiosu-forwarder.nro";
constexpr const char FORWARDER_PATH[] = "/config/nyf-aio-switch-updater/aiosu-forwarder.nro";

constexpr const char DAYBREAK_PATH[] = "/switch/daybreak.nro";

constexpr const char HIDDEN_AIO_FILE[] = "/config/nyf-aio-switch-updater/.nyf-aio-switch-updater";

constexpr const char LANG_PATH[] = "/switch/nyf-aio-switch-updater/lang";
constexpr const char LANG_EMBEDDED[] = "romfs:/lang/en.json";
// where the app update zip is unpacked before the forwarder moves the new nro into place
constexpr const char UPDATE_LANG_PATH[] = "/config/nyf-aio-switch-updater/switch/nyf-aio-switch-updater/lang";

constexpr const int LISTITEM_HEIGHT = 50;

enum class contentType
{
    custom,
    cheats,
    fw,
    app,
    bootloaders,
    ams_cfw,
    payloads,
    hekate_ipl,
};

constexpr std::string_view contentTypeNames[8]{"custom", "cheats", "firmwares", "app", "bootloaders", "cfws", "payloads", "hekate_ipl"};
