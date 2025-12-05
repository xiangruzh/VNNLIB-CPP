#pragma once

#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cerrno>
#include <cstring>

#include "Absyn.H"
#include "Parser.H"
#include "TypedBuilder.h"
#include "Printer.H"
#include "ParserError.H"
#include "VNNLibExport.h"

#include "Error.hpp"

VNNLIB_API std::unique_ptr<TQuery> parseQueryFile(std::string path);
VNNLIB_API std::unique_ptr<TQuery> parseQueryString(std::string content);
VNNLIB_API std::string checkQueryFile(std::string path);
VNNLIB_API std::string checkQueryString(std::string content);

