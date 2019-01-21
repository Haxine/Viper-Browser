#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QString>
#include <QSqlDatabase>

/**
 * @class DatabaseWorker
 * @brief Base class of all browser components that use their own database for
 *        storage of data and information.
 */
class DatabaseWorker
{
public:
    /**
     * @brief DatabaseWorker Constructs an object that interacts with a SQLite database
     * @param dbFile Full path of the database file
     * @param dbName Name of the database. If empty, will be the application's default database
     */
    explicit DatabaseWorker(const QString &dbFile, const QString &dbName = QString());

    /// Closes the database connection
    virtual ~DatabaseWorker();

    /// Executes the given query string, returning true on success, false on failure.
    bool exec(const QString &queryString);

protected:
    /// Returns true if the database contains the given table, false if else.
    bool hasTable(const QString &tableName);

protected:
    /// Returns true if the database contains the table structure(s) needed for it to function properly, false if else.
    virtual bool hasProperStructure() = 0;

    /// Sets initial table structure(s) of the database
    virtual void setup() = 0;

    /// Saves information to the database - called in destructor
    virtual void save() = 0;

    /// Loads records from the database
    virtual void load() = 0;

protected:
    /// Database object
    QSqlDatabase m_database;
};

#endif // DATABASEWORKER_H
