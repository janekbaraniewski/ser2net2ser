template <typename T>
inline Logger& Logger::operator<<(const T& msg) {
    stream << msg;
    return *this;
}

// Handle ostream manipulators like std::endl
inline Logger& Logger::operator<<(std::ostream& (*pf)(std::ostream&)) {
    stream << pf;
    return *this;
}
