#ifndef HISTORYTABLEMODEL_H
#define HISTORYTABLEMODEL_H

#include <vector>
#include <QAbstractTableModel>
#include <QDateTime>
#include <QMap>
#include <QPixmap>
#include <QUrl>

class HistoryManager;

/**
 * @struct HistoryTableItem
 * @brief Represents one item used by the \ref HistoryTableModel
 *        Items may be repeated several times in the table, so the data
 *        structure for an individual row, \ref HistoryTableRow will
 *        reference this structure by its index in a list
 */
struct HistoryTableItem
{
    /// Favicon of the page
    QPixmap Favicon;

    /// Title of the web page
    QString Title;

    /// URL of the page
    QString URL;
};

/**
 * @struct HistoryTableRow
 * @brief Represents an individual row of data in the \ref HistoryTableModel
 */
struct HistoryTableRow
{
    /// Index of the history table item
    int ItemIndex;

    /// Date/time of visit in string format
    QString VisitString;
};

/**
 * @class HistoryTableModel
 * @brief Loads browser history within a given range of dates into a table view
 */
class HistoryTableModel : public QAbstractTableModel
{
    Q_OBJECT

    friend class HistoryWidget;

public:
    /// Constructs the table model given a pointer to the HistoryManager and an optional parent object pointer
    explicit HistoryTableModel(HistoryManager *historyMgr, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /// Returns true if there is more data available for parent, otherwise returns false
    bool canFetchMore(const QModelIndex &parent) const override;

    /// Fetches any available data for the items with the parent specified by the parent index.
    void fetchMore(const QModelIndex &parent) override;

    /// Returns the data associated at the index with the given role
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

protected:
    /// Loads all history items beginning at the given date
    void loadFromDate(const QDateTime &date);

private:
    /// History manager
    HistoryManager *m_historyMgr;

    /// Date-time requested from the last call to loadFromDate(..) - when the date is older than 24 hours, it is loaded incrementially
    QDateTime m_targetDate;

    /// Date of the most recently loaded history item
    QDateTime m_loadedDate;

    /// Common history data
    std::vector<HistoryTableItem> m_commonData;

    /// List of visited history items, ordered by most to least recent visit
    std::vector<HistoryTableRow> m_history;
};

#endif // HISTORYTABLEMODEL_H
