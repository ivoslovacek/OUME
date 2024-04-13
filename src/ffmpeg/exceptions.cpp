#include "exceptions.hpp"

namespace OUMP {
UnreadableFileException::UnreadableFileException() noexcept
    : UnreadableFileException("File couldn't be opened") {}

UnreadableFileException::UnreadableFileException(std::string t_what) noexcept
    : m_what(t_what){};

UnreadableFileException::UnreadableFileException(
    UnreadableFileException& other) noexcept {
    this->m_what = other.m_what;
}

const char* UnreadableFileException::what() const noexcept {
    return this->m_what.c_str();
}
}  // namespace OUMP
