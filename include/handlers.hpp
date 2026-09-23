#pragma once

#include <string>

/**
 * Application Launcher — handlers
 *
 * Provides the main interface for working with installed
 * applications: loading, listing, searching and launching.
 *
 * Currently intended for terminal/console usage.
 * The interface is designed to be reusable for a future GUI.
 */

void load_app();
void list_app();
void open_app(std::string name);
void search_app(std::string query);