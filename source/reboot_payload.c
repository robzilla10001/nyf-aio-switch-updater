#include "reboot_payload.h"

#include "ams_bpc.h"

#define IRAM_PAYLOAD_MAX_SIZE 0x2F000

static alignas(0x1000) u8 g_reboot_payload[IRAM_PAYLOAD_MAX_SIZE];

static void inject_payload(void)
{
    printf("injecting\n");
    spsmInitialize();
    smExit();
    if (R_SUCCEEDED(amsBpcInitialize()) && R_SUCCEEDED(amsBpcSetRebootPayload(g_reboot_payload, 0x24000))) {
        spsmShutdown(true);
    }
}

int reboot_to_payload(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (f == NULL)
        return 0;

    fread(g_reboot_payload, 1, sizeof(g_reboot_payload), f);
    fclose(f);

    inject_payload();
    return 0;
}
