
#ifndef FORWARD_HPP_
#define FORWARD_HPP_

#include <vector>
#include <string>
#include <chrono>

void start(const std::vector<std::string>& interfaces, int start_core, int num_cores, std::chrono::seconds delay);

#endif
