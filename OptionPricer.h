// OptionPricer.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <vector>
#include <string>

#include <ctime>
#include <cpprest/http_client.h>

#include <cpprest/http_listener.h>
#include <cpprest/filestream.h>

#define _USE_MATH_DEFINES
#include <math.h> // For Black-scholes model
#include "includes/stock_types.h"
#include "includes/option_funcs.h"

#if defined(_WIN32) && !defined(__cplusplus_winrt)
// Extra includes for Windows desktop.
#include <windows.h>

#include <Shellapi.h>
#endif

// TODO: Reference additional headers your program requires here.

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams
using namespace web::http::oauth2::experimental;
using namespace web::http::experimental::listener;
