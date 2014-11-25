
#ifndef DROP_DEMO_SCHEDULER_HPP_
#define DROP_DEMO_SCHEDULER_HPP_

#include <string>

#include "activity/profile_scheduler.hpp"

namespace drop {
namespace activity {

class DemoScheduler: public ProfileScheduler
{
public:
    explicit DemoScheduler(const std::string& conf_file);
};

} // namespace activity
} // namespace drop

#endif
