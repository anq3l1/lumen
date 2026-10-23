#pragma once

#include <string>
#include <vector>

void addProcessTimes(const std::vector<std::string>& process_names, int seconds = 1);

void addLaunchesCount(const std::string& appnames, int launch = 0);

void rebuildAppStatistics();

int readTimeFromStatistic(const std::string& application_name);

int readLaunchFromStatistic(const std::string& appname);
