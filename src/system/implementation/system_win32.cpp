
#include "../system.hpp"

#include <cassert>
#include <cstdlib>

#include <QString>

namespace
{
    QString readVar(const char* name)
    {
        char* result = nullptr;
        size_t size = 0;

        _dupenv_s(&result, &size, name);

        return QString(result);
    }
}


QString System::getApplicationConfigDir()
{
    QString result(readVar("APPDATA"));

    result += "/broom";

    return result;
}


std::string System::findProgram(const std::string &)
{
    //TODO: use "where"
    assert(!"Not implemented");

    return "";
}


std::string System::userName()
{
    const std::string result = readVar("USERNAME").toStdString();

    return result;
}
