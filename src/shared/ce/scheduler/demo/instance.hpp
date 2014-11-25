
#ifndef DROP_INSTANCE_HPP_
#define DROP_INSTANCE_HPP_

namespace drop {
namespace activity {

struct ProfileScheduler;

} // namespace activity
} // namespace drop

extern "C" {

drop::activity::ProfileScheduler* create(const char* conf_file);
void destroy(drop::activity::ProfileScheduler* scheduler);

}

#endif
