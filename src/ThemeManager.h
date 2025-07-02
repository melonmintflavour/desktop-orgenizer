#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include <QObject> // For Q_NAMESPACE and Q_ENUM

// Using a namespace for a simple theme manager
namespace ThemeManager {
    // Q_NAMESPACE // Not strictly needed if not exposing to meta-object system directly from here
                 // but good practice if we were to make this a QObject later.

    enum class Theme {
        Light,
        Dark
    };
    // Q_ENUM(Theme) // Only if Q_NAMESPACE is used and we want string conversion via QMetaEnum

    void setCurrentTheme(Theme theme);
    Theme currentTheme();
    QString currentThemeStylesheet(); // Gets the full stylesheet string for the current theme
    void applyTheme(Theme theme); // Applies the theme to the application

    // Helper to load theme preference from QSettings
    Theme loadThemePreference();
    // Helper to save theme preference to QSettings
    void saveThemePreference(Theme theme);

    // Define stylesheet components here or load from resources
    QString getBaseStyle(); // Common styles
    QString getLightThemeStyle();
    QString getDarkThemeStyle();

} // namespace ThemeManager

#endif // THEMEMANAGER_H
