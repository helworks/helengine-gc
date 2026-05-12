#ifdef DrawText
#undef DrawText
#endif
#include "LogEntry.hpp"
#include "runtime/native_datetime.hpp"

LogEntry::LogEntry() : Level(), Message(), Timestamp()
{
}

::LogLevel LogEntry::get_Level()
{
return this->Level;
}

std::string LogEntry::get_Message()
{
return this->Message;
}

DateTime LogEntry::get_Timestamp()
{
return this->Timestamp;
}

LogEntry::LogEntry(::LogLevel level, std::string message, DateTime timestamp) : Level(), Message(), Timestamp()
{
this->Level = level;
this->Message = message;
this->Timestamp = timestamp;
}

