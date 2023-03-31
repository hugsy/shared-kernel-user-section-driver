#include "Log.hpp"


namespace Log
{

void
Log(_In_ const wchar_t* FormatString, ...)
{
    va_list args;
    wchar_t buffer[1024]{};
    va_start(args, FormatString);
    ::vswprintf_s(buffer, sizeof(buffer) / sizeof(wchar_t), FormatString, args);
    va_end(args);
    KdPrint(("%S", buffer));
}

void
ntperror(_In_ const wchar_t* prefix, _In_ NTSTATUS Status)
{
}

} // namespace Log
