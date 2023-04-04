#include "Log.hpp"


namespace Log
{

void
Log(_In_ const char* FormatString, ...)
{
    va_list args;
    char buffer[1024] {};
    va_start(args, FormatString);
    ::vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, FormatString, args);
    va_end(args);
}

} // namespace Log
