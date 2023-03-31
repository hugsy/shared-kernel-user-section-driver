#pragma once

#include "Utils.hpp"


Utils::KMutex::KMutex()
{
    ::KeInitializeMutex(&_mutex, 0);
}

void Utils::KMutex::Lock()
{
    ::KeWaitForSingleObject(&_mutex, Executive, KernelMode, false, nullptr);
}

void Utils::KMutex::Unlock()
{
    if (!::KeReleaseMutex(&_mutex, true))
    {
        ::KeWaitForSingleObject(&_mutex, Executive, KernelMode, false, nullptr);
    }
}

