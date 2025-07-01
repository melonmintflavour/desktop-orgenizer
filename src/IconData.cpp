#include "IconData.h"
#include <QFileInfo>

QString IconData::displayName() const
{
    return QFileInfo(m_filePath).fileName();
}

// Implement setCachedIcon and cachedIcon later if needed
// void IconData::setCachedIcon(const QPixmap& icon) { m_cachedIcon = icon; }
// QPixmap IconData::cachedIcon() const { return m_cachedIcon; }
