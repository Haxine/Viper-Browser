#ifndef FAVICONSTORAGE_H
#define FAVICONSTORAGE_H

#include "DatabaseWorker.h"
#include "LRUCache.h"

#include <map>
#include <memory>
#include <mutex>
#include <QHash>
#include <QIcon>
#include <QSet>
#include <QSqlQuery>
#include <QString>
#include <QUrl>

class NetworkAccessManager;
class QNetworkReply;

/// Stores information about a favicon
struct FaviconInfo
{
    /// The icon's FaviconID from the Favicons table (which stores the URL of the favicon on the host server)
    int iconID;

    /// The icon's DataID from the FaviconData table (used to access the icon)
    int dataID;

    /// The favicon
    QIcon icon;

    /// Set of URLs the user has visited in the most recent session that use the favicon
    QSet<QString> urlSet;
};

/**
 * @class FaviconStore
 * @brief Maintains a record of favicons from websites frequented by the user
 */
class FaviconStore : public QObject, private DatabaseWorker
{
    friend class DatabaseFactory;

    Q_OBJECT

public:
    /**
     * @brief FaviconStore Constructs the Favicon storage object
     * @param databaseFile Path of the favicon database file
     */
    FaviconStore(const QString &databaseFile, QObject *parent = nullptr);

    /// Destroys the favicon storage object, saving data to the favicon database
    virtual ~FaviconStore();

    /// Returns the favicon associated with the given URL if found in the database, otherwise
    /// returns an empty icon. If useCache is set to true, the url:icon mapping is stored in a LRUCache
    QIcon getFavicon(const QUrl &url, bool useCache = false);

    /**
     * @brief Attempts to update favicon for a specific URL in the database.
     * @param iconHRef The location in which the favicon is stored.
     * @param pageUrl The URL of the page displaying the favicon.
     * @param pageIcon The favicon on the page.
     */
    void updateIcon(const QString &iconHRef, const QUrl &pageUrl, QIcon pageIcon = QIcon());

private slots:
    /// Called after the request for a favicon has been completed
    void onReplyFinished();

private:
    /// Converts the given url into a string that is of a consistent format across the favicon storage system
    QString getUrlAsString(const QUrl &url) const;

    /// Saves the specific favicon with its URL and data structure into the database
    void saveToDB(const QString &faviconUrl, const FaviconInfo &favicon);

    /// Instantiates the stored query objects
    void setupQueries();

protected:
    /// Returns true if the favicon database contains the table structure(s) needed for it to function properly,
    /// false if else.
    bool hasProperStructure() override;

    /// Sets initial table structures of the database
    void setup() override;

    /// Saves information to the database
    void save() override;

    /// Loads records from the database
    void load() override;

private:
    /// Used to access prepared database queries
    enum class StoredQuery
    {
        InsertFavicon,
        InsertIconData,
        FindIconExactURL,
        FindIconLikeURL
    };

private:
    /// Network access manager - used to fetch favicons
    NetworkAccessManager *m_accessMgr;

    /// Network reply pointer for favicon requests
    QNetworkReply *m_reply;

    /// Hash map of favicon URLs to their data in a \ref FaviconInfo structure
    QHash<QString, FaviconInfo> m_favicons;

    /// Used when adding new records to the favicon table
    int m_newFaviconID;

    /// Used when adding new records to the favicon data table
    int m_newDataID;

    /// Map of query types to pointers of commonly used prepared statements
    std::map< StoredQuery, std::unique_ptr<QSqlQuery> > m_queryMap;

    /// Cache of most recently visited URLs and the icons associated with those pages
    LRUCache<std::string, QIcon> m_iconCache;

    /// Mutex
    mutable std::mutex m_mutex;
};

#endif // FAVICONSTORAGE_H
