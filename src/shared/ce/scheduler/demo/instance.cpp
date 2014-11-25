
#include "instance.hpp"

#include <exception>

#include "demo_scheduler.hpp"

drop::activity::ProfileScheduler* create(const char* conf_file)
{
    try
    {
        return new drop::activity::DemoScheduler(conf_file);
    }
    catch (...) { }

    return nullptr;
}

void destroy(drop::activity::ProfileScheduler* scheduler)
{
    delete scheduler;
}
