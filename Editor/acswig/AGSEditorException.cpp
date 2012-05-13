#include "AGSEditorException.h"

#include <stdexcept>

AGSEditorException::AGSEditorException(const std::string& msg) : std::runtime_error(msg) { }

