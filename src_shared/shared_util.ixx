module;

export module shared_util;
import logger;
import gregistry;

#include <format>
#include <string_view>

export void myprint_init(bool bSub = false)
{
    SetupLog();
}

// usage:  myprint("bla = {}", bla);  like std::format
export template<typename... Args>
void myprint(std::string_view fmt, Args&&... args)
{
    auto* console = RE::ConsoleLog::GetSingleton();
    if constexpr (sizeof...(args) == 0) {
        if (console) console->Print("%.*s", static_cast<int>(fmt.size()), fmt.data());
        logger::info("{}", fmt);
    } else {
        std::string message = std::vformat(fmt, std::make_format_args(args...));
        if (console) console->Print("%s", message.c_str());
        logger::info("{}", message);
    }
}
