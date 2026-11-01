#include "current_cfw.hpp"

#include <switch.h>

namespace CurrentCfw {

    std::string getAmsInfo()
    {
        u64 version;
        std::string res;
        if (R_SUCCEEDED(splGetConfig((SplConfigItem)65000, &version))) {
            res += std::to_string((version >> 56) & ((1 << 8) - 1)) + "." +
                   std::to_string((version >> 48) & ((1 << 8) - 1)) + "." +
                   std::to_string((version >> 40) & ((1 << 8) - 1));
            if (R_SUCCEEDED(splGetConfig((SplConfigItem)65007, &version)))
                res += version ? "|E" : "|S";
            return res;
        }
        else
            return "Couldn't retrieve AMS version";
    }

}  // namespace CurrentCfw
