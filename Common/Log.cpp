#include "Log.hpp"


namespace Log
{

void
log(_In_ const wchar_t* lpFormatString, ...)
{
    va_list args;
    WCHAR buffer[1024] = {
        0,
    };
    va_start(args, lpFormatString);
    ::vswprintf_s(buffer, sizeof(buffer) / sizeof(WCHAR), lpFormatString, args);
    va_end(args);
    ::KdPrint(("%S", buffer));
}

void
ntperror(_In_ const wchar_t* prefix, _In_ NTSTATUS Status)
{
}

} // namespace Log
