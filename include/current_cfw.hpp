#pragma once

#include <string>

namespace CurrentCfw {

    // "major.minor.micro|E" (emuMMC) or "|S" (sysMMC) of the running Atmosphère
    std::string getAmsInfo();

}  // namespace CurrentCfw
