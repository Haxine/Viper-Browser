#ifndef BookmarkWidget_H
#define BookmarkWidget_H

#include "BookmarkManager.h"
#include <memory>
#include <QUrl>
#include <QWidget>

namespace Ui {
class BookmarkWidget;
}

class QSortFilterProxyModel;

/**
 * @class BookmarkWidget
 * @brief Provides a graphical interface for managing the user's bookmarks
 */
class BookmarkWidget : public QWidget
{
    Q_OBJECT

    /// Options available from the import/export combo box
    enum class ComboBoxOption
    {
        NoAction   = 0,
        ImportHTML = 1,
        ExportHTML = 2
    };

public:
    explicit BookmarkWidget(QWidget *parent = 0);
    ~BookmarkWidget();

    /// Sets the pointer to the user's bookmark manager
    void setBookmarkManager(std::shared_ptr<BookmarkManager> bookmarkManager);

public slots:
    /// Reloads bookmark data into tree and table models
    void reloadBookmarks();

protected:
    /// Called when the bookmarks manager is closed
    virtual void closeEvent(QCloseEvent *event) override;

    /// Called to adjust the proportions of the columns belonging to the table view
    virtual void resizeEvent(QResizeEvent *event) override;

signals:
    /// Signal for the browser to open a bookmark onto the current web page
    void openBookmark(const QUrl &link);

    /// Signal for the browser to open a bookmark into a new tab
    void openBookmarkNewTab(const QUrl &link);

    /// Signal for the browser to open a bookmark into a new window
    void openBookmarkNewWindow(const QUrl &link);

    /// Called when the window is closed, signals the main window so that it can recreate the bookmarks menu
    void managerClosed();

private slots:
    /// Creates a context menu for the table view at the position of the cursor
    void onBookmarkContextMenu(const QPoint &pos);

    /// Creates a context menu for the folder view widget
    void onFolderContextMenu(const QPoint &pos);

    /// Called when the user changes their bookmark folder selection
    void onChangeFolderSelection(const QModelIndex &index);

    /// Called when the index of the active item has changed in the "Import/Export bookmarks from/to HTML" combo box
    void onImportExportBoxChanged(int index);

    /// Emits the openBookmark signal with the link parameter set to the URL of the bookmark requested to be opened
    void openInCurrentPage();

    /// Emits the openBookmarkNewTab signal with the link parameter set to the URL of the bookmark requested to be opened
    void openInNewTab();

    /// Emits the openBookmarkNewWindow signal with the link parameter set to the URL of the bookmark requested to be opened
    void openInNewWindow();

    /// Called by the add new bookmark action
    void addBookmark();

    /// Called by the add new folder action
    void addFolder();

    /// Deletes the current bookmark selection
    void deleteBookmarkSelection();

    /// Deletes the current folder selection (ignoring the root folder if root is included)
    void deleteFolderSelection();

    /// Called when the search bar is activated
    void searchBookmarks();

    /// Resets the bookmark folder model
    void resetFolderModel();

private:
    /// Returns a QUrl containing the location of the bookmark that the user has selected in the table view
    QUrl getUrlForSelection();

private:
    /// Dialog's user interface elements
    Ui::BookmarkWidget *ui;

    /// Pointer to the user's bookmark manager
    std::shared_ptr<BookmarkManager> m_bookmarkManager;

    /// Proxy model used for searching bookmarks
    QSortFilterProxyModel *m_proxyModel;
};

#endif // BookmarkWidget_H
