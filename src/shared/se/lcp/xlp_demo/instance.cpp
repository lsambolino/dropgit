
#include "instance.hpp"

#include <exception>

#include "gal_drop/local_control_policy_task.hpp"

#include "demo_lcp.hpp"

drop::LocalControlPolicyTask* create()
{
    try
    {
        return new drop::DemoLCP();
    }
    catch (...) { }

    return nullptr;
}

void destroy(drop::LocalControlPolicyTask* lcp)
{
    delete lcp;
}
