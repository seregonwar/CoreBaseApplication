#pragma once

namespace CoreNS {

/**
 * @brief Livelli di log supportati dal sistema
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERR = 3,
    CRITICAL = 4,
    FATAL = 5
};

} // namespace CoreNS
