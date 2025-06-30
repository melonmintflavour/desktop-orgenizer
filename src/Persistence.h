#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "DataModels.h"
#include "../third_party/nlohmann/json.hpp"
#include <string>
#include <fstream>
#include <vector>

// Use nlohmann::json
using json = nlohmann::json;

// --- Helper Macros for JSON Serialization/Deserialization ---
// These macros simplify the process of defining to_json and from_json functions for your structs.

// For simple structs/classes with public members
#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Type, ...) \
    inline void to_json(json& nlohmann_json_j, const Type& nlohmann_json_t) { \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) \
    } \
    inline void from_json(const json& nlohmann_json_j, Type& nlohmann_json_t) { \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__)) \
    }

// --- JSON Serialization for DataModels.h structs ---

namespace ns {
    // For ColorRGBA
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ColorRGBA, r, g, b, a)
    // For RectF
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(RectF, left, top, right, bottom)

    // For DesktopIcon
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DesktopIcon, id, name, path, iconPath, relativePosition)

    // For ZoneBackgroundType (enum)
    // nlohmann/json serializes enums as integers by default.
    // If string representation is desired, NLOHMANN_JSON_SERIALIZE_ENUM can be used.
    // For simplicity now, integer representation is fine.
    NLOHMANN_JSON_SERIALIZE_ENUM(ZoneBackgroundType, {
        {ZoneBackgroundType::Transparent, "Transparent"},
        {ZoneBackgroundType::SolidColor, "SolidColor"},
        {ZoneBackgroundType::Blurred, "Blurred"},
        {ZoneBackgroundType::Image, "Image"}
    })

    // For IconZone
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(IconZone, id, title, screenRect, icons, backgroundType, backgroundColor, backgroundImagePath, snapToGrid, gridRows, gridCols)

    // For DesktopPage
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DesktopPage, id, name, order, zones, wallpaperPath, usePageWallpaper)

    // For ApplicationSettings
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ApplicationSettings, currentTheme, pages, activePageId, version)

} // namespace ns (or global, matching DataModels.h)


class PersistenceManager {
public:
    PersistenceManager(const std::wstring& configFilePath) : filePath(configFilePath) {}

    bool SaveSettings(const ns::ApplicationSettings& settings) {
        json j = settings; // Uses the to_json functions defined via macros
        try {
            std::ofstream file(filePath);
            if (!file.is_open()) {
                // Consider logging an error here
                return false;
            }
            file << j.dump(4); // Pretty print with 4 spaces
            file.close();
            return true;
        } catch (const std::exception& e) {
            // Consider logging e.what()
            return false;
        }
    }

    bool LoadSettings(ns::ApplicationSettings& settings) {
        try {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                // Config file might not exist on first run, which is fine.
                // Initialize with default settings.
                settings = ns::ApplicationSettings(); // Initialize with defaults
                SaveSettings(settings); // Optionally save the defaults
                return true; // Or false if you expect it to always exist after first run
            }

            json j;
            file >> j;
            file.close();

            // Handle versioning: if the loaded version is older, migrate settings.
            // For now, just directly deserialize.
            // int loaded_version = j.value("version", 0);
            // if (loaded_version < settings.version) { /* migrate */ }

            settings = j.get<ns::ApplicationSettings>(); // Uses from_json functions

            // Ensure at least one page exists after loading
            if (settings.pages.empty()) {
                ns::DesktopPage defaultPage;
                defaultPage.id = L"default_page_1_after_load";
                defaultPage.name = L"My Desktop";
                defaultPage.order = 0;
                settings.pages.push_back(defaultPage);
                if (settings.activePageId.empty()) {
                    settings.activePageId = defaultPage.id;
                }
            }
             if (settings.activePageId.empty() && !settings.pages.empty()) {
                settings.activePageId = settings.pages[0].id;
            }


            return true;
        } catch (const json::parse_error& e) {
            // Handle parse error, e.g., corrupted file. Load defaults.
            // Consider logging e.what()
            settings = ns::ApplicationSettings(); // Initialize with defaults
            SaveSettings(settings); // Overwrite corrupted/old file with defaults
            return false; // Indicate that loading failed and defaults were used
        } catch (const json::type_error& e) {
            // Handle type error (e.g. missing fields, wrong types). Load defaults.
            // Consider logging e.what()
            settings = ns::ApplicationSettings();
            SaveSettings(settings);
            return false;
        }
        catch (const std::exception& e) {
            // Generic error
            // Consider logging e.what()
            settings = ns::ApplicationSettings();
            SaveSettings(settings);
            return false;
        }
    }

private:
    std::wstring filePath;
};

#endif // PERSISTENCE_H
