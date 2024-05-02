//
// Created by jonah on 5/1/2024.
//

#include "EngineException.h"
#include <sstream>

EngineException::EngineException(int line, const char *file) noexcept :
        line(line), file(file)
{}

const char *EngineException::what() const noexcept {
    exception::what();
    std::ostringstream oss;
    oss << GetType() << '\n' << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char *EngineException::GetType() const noexcept {
    return "Engine Exception";
}

int EngineException::GetLine() const noexcept {
    return line;
}

const std::string &EngineException::GetFile() const noexcept {
    return file;
}

std::string EngineException::GetOriginString() const noexcept {
    std::ostringstream oss;
    oss << "Line: " << line << "In File: " << file << std::endl;
    return oss.str();
}
