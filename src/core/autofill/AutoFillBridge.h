#ifndef AUTOFILLBRIDGE_H
#define AUTOFILLBRIDGE_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

class WebPage;

/**
 * @class AutoFillBridge
 * @brief Acts as a bridge between the AutoFill system and web content
 *        through a WebChannel
 */
class AutoFillBridge : public QObject
{
    Q_OBJECT

public:
    /// Constructs the AutoFillBridge with the given parent
    explicit AutoFillBridge(WebPage *parent);

    /// AutoFillBridge destructor
    ~AutoFillBridge();

public slots:
    /**
     * @brief Triggered by a \ref WebPage when a form was submitted
     * @param pageUrl The URL of the page containing the form
     * @param username The username field of the form
     * @param password The password entered by the user
     * @param formData All of the key-value pairs entered into the form
     */
    void onFormSubmitted(const QString &pageUrl, const QString &username, const QString &password, const QMap<QString, QVariant> &formData);

private:
    /// Pointer to the web page that owns this bridge
    WebPage *m_page;
};

#endif // AUTOFILLBRIDGE_H
