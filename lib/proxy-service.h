/*
 * This file contains D-Bus client proxy classes generated by qt-client-gen.py.
 *
 * This file can be distributed under the same terms as the specification from
 * which it was generated.
 */

#ifndef OTR_PROXY_SERVICE_HEADER
#define OTR_PROXY_SERVICE_HEADER

#include <TelepathyQt/Types>

#include <QtGlobal>

#include <QString>
#include <QObject>
#include <QVariant>

#include <QDBusPendingReply>

#include <TelepathyQt/AbstractInterface>
#include <TelepathyQt/DBusProxy>
#include <TelepathyQt/Global>

namespace Tp
{
class PendingVariant;
class PendingOperation;
}

namespace Tp
{
namespace Client
{

/**
 * \class ProxyServiceInterface
 * \headerfile TelepathyQt/proxy-service.h <TelepathyQt/ProxyService>
 * \ingroup clientproxyservice
 *
 * Proxy class providing a 1:1 mapping of the D-Bus interface "org.kde.TelepathyProxy.ProxyService".
 */
class TP_QT_EXPORT ProxyServiceInterface : public Tp::AbstractInterface
{
    Q_OBJECT

public:
    /**
     * Returns the name of the interface "org.kde.TelepathyProxy.ProxyService", which this class
     * represents.
     *
     * \return The D-Bus interface name.
     */
    static inline QLatin1String staticInterfaceName()
    {
        return QLatin1String("org.kde.TelepathyProxy.ProxyService");
    }

    /**
     * Creates a ProxyServiceInterface associated with the given object on the session bus.
     *
     * \param busName Name of the service the object is on.
     * \param objectPath Path to the object on the service.
     * \param parent Passed to the parent class constructor.
     */
    ProxyServiceInterface(
        const QString& busName,
        const QString& objectPath,
        QObject* parent = 0
    );

    /**
     * Creates a ProxyServiceInterface associated with the given object on the given bus.
     *
     * \param connection The bus via which the object can be reached.
     * \param busName Name of the service the object is on.
     * \param objectPath Path to the object on the service.
     * \param parent Passed to the parent class constructor.
     */
    ProxyServiceInterface(
        const QDBusConnection& connection,
        const QString& busName,
        const QString& objectPath,
        QObject* parent = 0
    );

    /**
     * Creates a ProxyServiceInterface associated with the same object as the given proxy.
     *
     * \param proxy The proxy to use. It will also be the QObject::parent()
     *               for this object.
     */
    ProxyServiceInterface(Tp::DBusProxy *proxy);

    /**
     * Creates a ProxyServiceInterface associated with the same object as the given proxy.
     * Additionally, the created proxy will have the same parent as the given
     * proxy.
     *
     * \param mainInterface The proxy to use.
     */
    explicit ProxyServiceInterface(const Tp::AbstractInterface& mainInterface);

    /**
     * Creates a ProxyServiceInterface associated with the same object as the given proxy.
     * However, a different parent object can be specified.
     *
     * \param mainInterface The proxy to use.
     * \param parent Passed to the parent class constructor.
     */
    ProxyServiceInterface(const Tp::AbstractInterface& mainInterface, QObject* parent);

    /**
     * Request all of the DBus properties on the interface.
     *
     * \return A pending variant map which will emit finished when the properties have
     *          been retrieved.
     */
    Tp::PendingVariantMap *requestAllProperties() const
    {
        return internalRequestAllProperties();
    }

Q_SIGNALS:
    /**
     * Represents the signal \c ProxyConnected on the remote object.
     * 
     * Signals that a proxy has been connected
     *
     * \param proxy
     *     
     *     The object path of the connected proxy
     */
    void ProxyConnected(const QDBusObjectPath& proxy);

    /**
     * Represents the signal \c ProxyDisconnected on the remote object.
     * 
     * Signals that a proxy has been disconnected
     *
     * \param proxy
     *     
     *     The object path of the disconnectd proxy type
     */
    void ProxyDisconnected(const QDBusObjectPath& proxy);

protected:
    virtual void invalidate(Tp::DBusProxy *, const QString &, const QString &);
};
}
}
Q_DECLARE_METATYPE(Tp::Client::ProxyServiceInterface*)

#endif
