#include "ThemeManager.h"
#include <QApplication>
#include <QSettings>
#include <QStyle>
#include <QDebug>

namespace ThemeManager {

    static Theme s_currentTheme = Theme::Light; // Default to light
    const QString SETTINGS_THEME_KEY = "Appearance/Theme";

    void saveThemePreference(Theme theme) {
        QSettings settings; // Uses organizationName and applicationName
        settings.setValue(SETTINGS_THEME_KEY, theme == Theme::Dark ? "dark" : "light");
        qDebug() << "Theme preference saved:" << (theme == Theme::Dark ? "dark" : "light");
    }

    Theme loadThemePreference() {
        QSettings settings;
        QString themeStr = settings.value(SETTINGS_THEME_KEY, "light").toString();
        if (themeStr.toLower() == "dark") {
            qDebug() << "Theme preference loaded: dark";
            return Theme::Dark;
        }
        qDebug() << "Theme preference loaded: light";
        return Theme::Light;
    }

    void setCurrentTheme(Theme theme) {
        if (s_currentTheme != theme) {
            s_currentTheme = theme;
            applyTheme(theme); // Apply immediately
            saveThemePreference(theme);
        }
    }

    Theme currentTheme() {
        return s_currentTheme;
    }

    void applyTheme(Theme theme) {
        s_currentTheme = theme; // Ensure static variable is also set
        QString styleSheet = currentThemeStylesheet();
        qApp->setStyleSheet(styleSheet);
        qDebug() << "Applied theme:" << (theme == Theme::Dark ? "Dark" : "Light");

        // Optionally, to force re-polish of all widgets if some styles don't update:
        // for (QWidget *widget : qApp->allWidgets()) {
        //     widget->style()->unpolish(widget);
        //     widget->style()->polish(widget);
        //     widget->update();
        // }
    }

    QString getBaseStyle() {
        // Common styles for both themes
        return QString(R"(
            QWidget {
                font-family: Segoe UI, Arial, sans-serif; /* Example font */
            }
            QToolTip {
                border: 1px solid #76797C; /* Slightly lighter for dark, darker for light */
                padding: 4px;
                background-color: #3E3E3E; /* Default dark, override in light */
                color: #E0E0E0;
                border-radius: 3px;
            }
            QMenuBar {
                background-color: transparent; /* Will be part of pageControlsWidget style or window style */
            }
            QMenuBar::item {
                padding: 4px 8px;
                background: transparent;
            }
            QMenuBar::item:selected {
                background: rgba(0,0,0,0.1); /* Subtle selection */
            }
        )");
    }

    QString getLightThemeStyle() {
        return getBaseStyle() + QString(R"(
            /* Global Window Text (if not overridden) */
            QWidget { color: #1E1E1E; }

            QMainWindow { /* background-color: #F3F3F3; */ } /* Main window is transparent */

            QWidget#pageControlsWidget {
                background-color: rgba(235, 235, 235, 0.85); /* Lighter, more opaque */
                border-radius: 6px;
            }
            QMenuBar { color: #1E1E1E; }
            QMenuBar::item:selected { background-color: #D0D0D0; }


            QTabWidget::pane { border: none; background: transparent; }
            QTabBar::tab {
                background: transparent;
                color: #454545;
                padding: 8px 12px;
                border: none; /* Minimalist - no border */
                border-bottom: 2px solid transparent; /* Underline for selected */
                min-width: 90px;
            }
            QTabBar::tab:selected {
                color: #007AFF; /* Accent color for selected tab text */
                border-bottom: 2px solid #007AFF; /* Accent underline */
                font-weight: bold;
            }
            QTabBar::tab:hover:!selected {
                color: #005DD1;
                background-color: rgba(0,0,0,0.05);
            }
            QTabBar::close-button {
                /* Ideally use an SVG or font icon for themeability */
                /* For now, rely on default or hide if too ugly */
                 margin-left: 4px;
            }

            QPushButton {
                background-color: #E0E0E0;
                color: #252525;
                border: 1px solid #C0C0C0;
                padding: 6px 10px;
                border-radius: 4px;
                font-size: 9pt;
            }
            QPushButton:hover { background-color: #D5D5D5; border-color: #B0B0B0; }
            QPushButton:pressed { background-color: #C5C5C5; }

            QPushButton#addPageButton { /* The '+' button */
                background-color: #007AFF;
                color: white;
                font-size: 14pt; font-weight: bold;
                border: none;
                padding: 0px; /* Adjust for visual centering of '+' */
                border-radius: 14px;
                min-width: 28px; max-width: 28px; min-height: 28px; max-height: 28px;
            }
            QPushButton#addPageButton:hover { background-color: #0062CC; }
            QPushButton#addPageButton:pressed { background-color: #0052AA; }

            QPushButton#addZoneButton { /* Standard button look from above */
                 margin-left: 5px;
            }

            ZoneWidget {
                /* ZoneWidget background is per-instance. Text color for title: */
                color: #FFFFFF; /* Default title color, assuming darkish zone backgrounds */
            }
            IconWidget {
                color: #F0F0F0; /* Default icon text color on light theme (assuming icons on zones) */
            }
            IconWidget[selected="true"] { /* Example for future selection */
                border: 1px solid #007AFF;
            }


            QMenu { background-color: #FFFFFF; border: 1px solid #D0D0D0; color: #252525; }
            QMenu::item { padding: 6px 24px; }
            QMenu::item:selected { background-color: #E8E8E8; }
            QMenu::separator { height: 1px; background: #E0E0E0; margin: 4px 0px; }

            QInputDialog { color: #252525; background-color: #FDFDFD; }
            QMessageBox { color: #252525; background-color: #FDFDFD; }
            QLineEdit { background-color: #FFFFFF; color: #252525; border: 1px solid #C0C0C0; padding: 4px; border-radius: 3px; }
            QToolTip { background-color: #F0F0F0; color: #252525; border-color: #C0C0C0; }
        )");
    }

    QString getDarkThemeStyle() {
        return getBaseStyle() + QString(R"(
            /* Global Window Text (if not overridden) */
            QWidget { color: #E0E0E0; }

            QMainWindow { /* background-color: #1E1E1E; */ } /* Main window is transparent */

            QWidget#pageControlsWidget {
                background-color: rgba(30, 30, 30, 0.85); /* Darker, more opaque */
                border-radius: 6px;
            }
            QMenuBar { color: #E0E0E0; }
            QMenuBar::item:selected { background-color: #424242; }

            QTabWidget::pane { border: none; background: transparent; }
            QTabBar::tab {
                background: transparent;
                color: #AAAAAA;
                padding: 8px 12px;
                border: none;
                border-bottom: 2px solid transparent;
                min-width: 90px;
            }
            QTabBar::tab:selected {
                color: #3391FF; /* Lighter blue for dark theme */
                border-bottom: 2px solid #3391FF;
                font-weight: bold;
            }
            QTabBar::tab:hover:!selected {
                color: #CCCCCC;
                background-color: rgba(255,255,255,0.05);
            }
            QTabBar::close-button {
                 /* image: url(:/icons/close-tab-dark.svg); */ /* Use SVG for themable icons */
                 margin-left: 4px;
            }

            QPushButton {
                background-color: #3E3E3E;
                color: #E0E0E0;
                border: 1px solid #505050;
                padding: 6px 10px;
                border-radius: 4px;
                font-size: 9pt;
            }
            QPushButton:hover { background-color: #4A4A4A; border-color: #606060; }
            QPushButton:pressed { background-color: #303030; }

            QPushButton#addPageButton { /* The '+' button */
                background-color: #3391FF;
                color: #1E1E1E; /* Dark text on light blue */
                font-size: 14pt; font-weight: bold;
                border: none;
                padding: 0px;
                border-radius: 14px;
                min-width: 28px; max-width: 28px; min-height: 28px; max-height: 28px;
            }
            QPushButton#addPageButton:hover { background-color: #2978CC; }
            QPushButton#addPageButton:pressed { background-color: #2060A0; }

            QPushButton#addZoneButton { /* Standard button look from above */
                 margin-left: 5px;
            }

            ZoneWidget {
                /* ZoneWidget background is per-instance. Text color for title: */
                color: #1E1E1E; /* Default title color, assuming lightish zone backgrounds */
            }
            IconWidget {
                color: #252525; /* Default icon text color on dark theme (assuming icons on zones) */
            }
            IconWidget[selected="true"] { /* Example for future selection */
                border: 1px solid #3391FF;
            }

            QMenu { background-color: #2B2B2B; border: 1px solid #3C3C3C; color: #E0E0E0; }
            QMenu::item { padding: 6px 24px; }
            QMenu::item:selected { background-color: #424242; }
            QMenu::separator { height: 1px; background: #3C3C3C; margin: 4px 0px; }

            QInputDialog { color: #E0E0E0; background-color: #252525; }
            QMessageBox { color: #E0E0E0; background-color: #252525; }
            QLineEdit { background-color: #1E1E1E; color: #E0E0E0; border: 1px solid #505050; padding: 4px; border-radius: 3px; }
            /* QToolTip uses base style, already dark */
        )");
    }

    QString currentThemeStylesheet() {
        if (s_currentTheme == Theme::Dark) {
            return getDarkThemeStyle();
        }
        return getLightThemeStyle();
    }

} // namespace ThemeManager
