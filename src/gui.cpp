#include <gdkmm/texture.h>
#include <gtkmm.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <time.h>

#include "gui.hpp"
#include "handlers.hpp"
#include "paths.hpp"
#include "stats.hpp"

namespace fs = std::filesystem;

namespace
{
std::string lower(std::string value)
{
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}

fs::path get_random_wallpaper()
{
    const fs::path directory =
        fs::path(std::getenv("HOME")) / ".config/lumen/Wallpaper";
    
    if (!fs::exists(directory) || !fs::is_directory(directory))
        return {};

    std::vector<fs::path> wallpapers;

    for (const auto& entry : fs::directory_iterator(directory))
    {
        if (!entry.is_regular_file())
            continue;

        const auto ext = lower(entry.path().extension().string());

        if (ext == ".png" || ext == ".jpg" ||
            ext == ".jpeg" || ext == ".webp")
        {
            wallpapers.push_back(entry.path());
        }
    }

    if (wallpapers.empty())
        return {};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dist(
        0, wallpapers.size() - 1
    );

    return wallpapers[dist(gen)].string();
}

fs::path get_css_path()
{
    const auto configured = lumen_paths::style_file();
    if (fs::exists(configured))
        return configured;

    std::error_code ec;
    const auto executable = fs::read_symlink("/proc/self/exe", ec);
    if (!ec)
    {
        const auto beside_executable = executable.parent_path() / "style.css";
        if (fs::exists(beside_executable))
            return beside_executable;

        const auto source_root_style = executable.parent_path().parent_path() / "style.css";
        if (fs::exists(source_root_style))
            return source_root_style;
    }

    return configured;
}

std::string get_category(const Application& app)
{
    return app.category.empty() ? "Uncategorized" : app.category;
}

std::string format_duration(double seconds)
{
    if (seconds < 0)
        seconds = 0;

    const auto total = static_cast<long long>(seconds);
    const auto hours = total / 3600;
    const auto minutes = (total % 3600) / 60;

    std::ostringstream output;

    if (hours > 0)
        output << hours << " h " << minutes << " min";
    else if (minutes > 0)
        output << minutes << " min";
    else
        output << total << " sec";

    return output.str();
}

std::string get_usage_time(const Application& app)
{
    return format_duration(static_cast<double>(readTimeFromStatistic(app.name)));
}

std::string get_launch(const Application& app)
{
    return std::to_string(readLaunchFromStatistic(app.name));
}

void set_image(Gtk::Image& image, const std::string& icon)
{
    image.set_from_icon_name("application-x-executable");

    if (icon.empty())
        return;

    if (icon.front() == '/')
    {
        try
        {
            image.set(Gdk::Texture::create_from_filename(icon));
        }
        catch (const Glib::Error& error)
        {
            std::cerr << "Failed to load icon " << icon
                      << ": " << error.what() << '\n';
        }
    }
    else
    {
        image.set_from_icon_name(icon);
    }
}
}

class MainWindow : public Gtk::ApplicationWindow
{
public:
    void show_home()
    {
        wallpaper.set_visible(true);
        dim_overlay.set_visible(true);
        clock_label.set_visible(true);
        date_label.set_visible(true);
        details_header.set_visible(false);
    }

    void update_clock()
    {
        const std::time_t now = std::time(nullptr);
        std::tm local_time{};

        localtime_r(&now, &local_time);

        std::ostringstream output;
        output << std::put_time(&local_time, "%H:%M");

        clock_label.set_text(output.str());
    }

    void update_date()
    {
        time_t now = time(nullptr);
        tm* local = localtime(&now);

        char date[32];
        strftime(date, sizeof(date), "%d.%m.%Y", local);

        date_label.set_text(date);
    }

    MainWindow()
    {
        set_title("Lumen");
        set_default_size(900, 600);
        show_home();

        load_css();

        main_box.set_orientation(Gtk::Orientation::HORIZONTAL);
        main_box.add_css_class("main-window");
        main_box.set_spacing(0);
        main_box.set_margin(0);

        sidebar.set_orientation(Gtk::Orientation::VERTICAL);
        sidebar.set_size_request(240, -1);
        sidebar.set_spacing(10);
        sidebar.set_margin(12);
        sidebar.add_css_class("sidebar");

        search.set_placeholder_text("Search applications...");
        search.add_css_class("app-search");
        sidebar.append(search);

        app_list.set_selection_mode(Gtk::SelectionMode::NONE);
        app_list.add_css_class("app-list");

        app_scroll.set_child(app_list);
        app_scroll.set_policy(
            Gtk::PolicyType::NEVER,
            Gtk::PolicyType::AUTOMATIC
        );
        app_scroll.set_vexpand(true);
        sidebar.append(app_scroll);

        wallpaper.set_filename(
            get_random_wallpaper()
        );

        wallpaper.set_can_shrink(true);
        wallpaper.set_content_fit(Gtk::ContentFit::COVER);

        content_overlay.set_child(wallpaper);

        dim_overlay.set_hexpand(true);
        dim_overlay.set_vexpand(true);
        dim_overlay.set_halign(Gtk::Align::FILL);
        dim_overlay.set_valign(Gtk::Align::FILL);
        dim_overlay.add_css_class("wallpaper-dim");

        content_overlay.add_overlay(dim_overlay);

        clock_label.set_halign(Gtk::Align::FILL);
        clock_label.set_valign(Gtk::Align::FILL);
        clock_label.set_hexpand(true);
        clock_label.set_vexpand(true);
        clock_label.set_halign(Gtk::Align::FILL);
        clock_label.set_justify(Gtk::Justification::CENTER);
        clock_label.set_xalign(0.5f);
        clock_label.set_yalign(0.4f);
        clock_label.set_margin(10);
        clock_label.add_css_class("home-clock");

        date_label.set_halign(Gtk::Align::FILL);
        date_label.set_valign(Gtk::Align::FILL);
        date_label.set_hexpand(true);
        date_label.set_vexpand(true);
        date_label.set_halign(Gtk::Align::FILL);
        date_label.set_justify(Gtk::Justification::CENTER);
        date_label.set_xalign(0.5f);
        date_label.set_yalign(0.55f);
        date_label.set_margin(10);
        date_label.add_css_class("date");

        content_overlay.add_overlay(clock_label);
        content_overlay.add_overlay(date_label);

        update_clock();
        update_date();

        Glib::signal_timeout().connect(
            [this]()
            {
                update_clock();
                update_date();
                return true;
            },
            1000
        );

        content.set_hexpand(true);
        content.set_vexpand(true);
        content.set_spacing(10);
        content.set_margin(15);
        content.add_css_class("content");

        content_overlay.add_overlay(content);

        details_header.set_orientation(Gtk::Orientation::HORIZONTAL);
        details_header.set_spacing(20);
        details_header.set_valign(Gtk::Align::START);
        details_header.add_css_class("details-header");
        details_header.set_visible(false);

        details_icon.set_pixel_size(64);
        details_icon.set_valign(Gtk::Align::START);
        details_icon.set_from_icon_name("application-x-executable");

        details_info.set_orientation(Gtk::Orientation::VERTICAL);
        details_info.set_spacing(8);
        details_info.set_valign(Gtk::Align::CENTER);

        details_name.set_text("");
        details_name.set_xalign(0.0f);
        details_name.set_wrap(true);
        details_name.add_css_class("page-title");

        details_category.set_text("Category: —");
        details_category.set_xalign(0.0f);
        details_category.add_css_class("detail-label");

        details_usage.set_text("Usage time: —");
        details_usage.set_xalign(0.0f);
        details_usage.add_css_class("detail-label");

        details_launches.set_text("Launches: —");
        details_launches.set_xalign(0.0f);
        details_launches.add_css_class("detail-label");


        details_info.append(details_name);
        details_info.append(details_category);
        details_info.append(details_usage);
        details_info.append(details_launches);

        details_header.append(details_icon);
        details_header.append(details_info);

        content.append(details_header);

        main_box.append(sidebar);
        main_box.append(content_overlay);
        set_child(main_box);

        load_app();
        rebuild_app_list();

        search.signal_search_changed().connect([this]() {
            rebuild_app_list(search.get_text());
        });
    }

private:
    Glib::RefPtr<Gtk::CssProvider> css_provider;

    Gtk::Box main_box{Gtk::Orientation::HORIZONTAL};
    Gtk::Box sidebar{Gtk::Orientation::VERTICAL};
    Gtk::Box content{Gtk::Orientation::VERTICAL};
    Gtk::Overlay content_overlay;
    Gtk::Picture wallpaper;
    Gtk::Box dim_overlay{Gtk::Orientation::VERTICAL};
    Gtk::Label clock_label;
    Gtk::Label date_label;

    Gtk::Box details_header{Gtk::Orientation::HORIZONTAL};
    Gtk::Box details_info{Gtk::Orientation::VERTICAL};

    Gtk::SearchEntry search;
    Gtk::ScrolledWindow app_scroll;
    Gtk::ListBox app_list;

    Gtk::Image details_icon;
    Gtk::Label details_name;
    Gtk::Label details_category;
    Gtk::Label details_usage;
    Gtk::Label details_launches;

    void load_css()
    {
        css_provider = Gtk::CssProvider::create();

        css_provider->signal_parsing_error().connect(
            [](const Glib::RefPtr<const Gtk::CssSection>&,
               const Glib::Error& error)
            {
                std::cerr << "CSS parsing error: "
                          << error.what() << '\n';
            }
        );

        const auto path = get_css_path();

        try
        {
            css_provider->load_from_path(path.string());

            auto display = Gdk::Display::get_default();

            if (display)
            {
                Gtk::StyleContext::add_provider_for_display(
                    display,
                    css_provider,
                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
                );
            }

            std::cout << "Loaded CSS: " << path << '\n';
        }
        catch (const Glib::Error& error)
        {
            std::cerr << "CSS loading error: "
                      << error.what() << '\n';
        }
    }

    void show_app_details(const Application& app)
    {
        details_name.set_text(app.name);

        details_category.set_text(
            "Category: " + get_category(app)
        );

        details_usage.set_text(
            "Usage time: " + get_usage_time(app)
        );

        details_launches.set_text(
            "Launches: " + get_launch(app)
        );

        set_image(details_icon, app.icon);

        details_header.set_visible(true);
        wallpaper.set_visible(false);
        dim_overlay.set_visible(false);
        clock_label.set_visible(false);
        date_label.set_visible(false);
    }

    void rebuild_app_list(const std::string& query = "")
    {
        while (auto* child = app_list.get_first_child())
            app_list.remove(*child);

        const std::string needle = lower(query);
        std::vector<const Application*> matches;

        matches.reserve(applications.size());

        for (const auto& app : applications)
        {
            if (needle.empty() ||
                lower(app.name).find(needle) != std::string::npos)
            {
                matches.push_back(&app);
            }
        }

        std::sort(
            matches.begin(),
            matches.end(),
            [](const Application* a, const Application* b) {
                return lower(a->name) < lower(b->name);
            }
        );

        for (const Application* app : matches)
        {
            auto* row = Gtk::make_managed<Gtk::Box>(
                Gtk::Orientation::HORIZONTAL, 10
            );

            row->set_halign(Gtk::Align::FILL);
            row->set_hexpand(true);
            row->set_margin(2);
            row->set_focusable(true);
            row->set_tooltip_text(
                app->name + "\nSingle click: details\nDouble click: launch"
            );
            row->add_css_class("app-button");

            auto* image = Gtk::make_managed<Gtk::Image>();
            image->set_pixel_size(24);
            set_image(*image, app->icon);
            row->append(*image);

            auto* label = Gtk::make_managed<Gtk::Label>(app->name);
            label->set_xalign(0.0f);
            label->set_hexpand(true);
            label->set_ellipsize(Pango::EllipsizeMode::END);
            row->append(*label);

            auto gesture = Gtk::GestureClick::create();
            gesture->set_button(1);

            gesture->signal_pressed().connect(
                [this, app](int n_press, double, double)
                {
                    if (n_press == 1)
                    {
                        show_app_details(*app);
                    }
                    else if (n_press == 2)
                    {
                        open_app(app->name);
                        close();
                    }
                }
            );

            row->add_controller(gesture);
            app_list.append(*row);
        }
    }
};

int run_gui(int argc, char* argv[])
{
    auto app = Gtk::Application::create("com.anq3l1.lumen");
    return app->make_window_and_run<MainWindow>(argc, argv);
}
