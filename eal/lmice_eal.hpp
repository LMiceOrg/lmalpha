#ifndef LMICE_EAL_HPP
#define LMICE_EAL_HPP

/// C++ version eal header

extern "C"
{

// Tracer
#ifndef LMICE_EAL_NO_TRACER
#include "lmice_trace.h"
#endif

// Logger
#ifndef LMICE_EAL_NO_LOGGER
#include "lmice_eal_log.h"
#endif

}

#endif // LMICE_EAL_HPP
