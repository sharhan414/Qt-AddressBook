#include "clientmanager.h"
#include <QTimer>
#include <QDataStream>
#include "package.h"
#include "contact.h"

using namespace net;
ClientManager::ClientManager(const ConnectionArgumentsParser& connectionSettings)
    : m_rConnectionSettings {connectionSettings}
{
    connectSocketSignals();
    connectToServer();
}

bool ClientManager::sendPackage(const Package& package)
{
    const bool isConnected {m_state == ConnectionState::Connected};
    if (isConnected)
    {
        m_serverSocket.write(package.rawData());
    }
    return isConnected;
}

void ClientManager::onSocketError(QAbstractSocket::SocketError socketError)
{
    qCritical() << "Socket error happened: " << socketError;

    // Each time socket error was recieved, we try to connect to server again.
    // If connection was failed, we'll get in this slot again.
    QTimer::singleShot(reconnectionTime(), this,
                            &ClientManager::connectToServer);

    emit reconnectionTimerStarted();
}

void ClientManager::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "Socket state changed: " << socketState;

    switch (socketState) {
        case QAbstractSocket::SocketState::BoundState:
        case QAbstractSocket::SocketState::ConnectedState: {
            m_state = ConnectionState::Connected;
            break;
        }
        case QAbstractSocket::SocketState::ClosingState:
        case QAbstractSocket::SocketState::UnconnectedState: {
            m_state = ConnectionState::Disconnected;
            break;
        }
        default: {
            m_state = ConnectionState::Connecting;
        }
    }

    emit connectionStateChanged(m_state);
}

void ClientManager::onDataRecieved()
{
    QDataStream in {&m_serverSocket};
    in.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    forever {
        in.startTransaction();

        Package inputPackage;
        in >> inputPackage;

        if (!in.commitTransaction()) {
            break;
        }

        qDebug() << "Recieved package with type " << static_cast<int>(inputPackage.type());

        switch(inputPackage.type())
        {
            case PackageType::CONTACTS_RESPONSE:
            {
                handleContactsResponsePackage(inputPackage);
                break;
            }
            default: {
                qDebug() << "Invalid package!";
            }
        }
    }
}

void ClientManager::connectToServer()
{
    qDebug() << "Connecting to server...";

    m_serverSocket.connectToHost({m_rConnectionSettings.hostArgument()},
                           static_cast<quint16>(m_rConnectionSettings.portArgument().toUInt()));
}

void ClientManager::connectSocketSignals()
{
    connect(&m_serverSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
                this, &ClientManager::onSocketError);
    connect(&m_serverSocket, &QAbstractSocket::stateChanged,
                this, &ClientManager::onSocketStateChanged);
    connect(&m_serverSocket, &QAbstractSocket::readyRead,
                this, &ClientManager::onDataRecieved);

}

std::vector<Contact> transform(const std::vector<QVariant>& source)
{
    std::vector<Contact> target;
    std::transform(source.begin(), source.end(), std::back_inserter(target),
                   [](const QVariant& entry) {
        QSequentialIterable value = entry.value<QSequentialIterable>();
        return Contact {value.at(1).toString(), value.at(2).toString(), value.at(3).toString(), value.at(0).toInt()};
    });
    return target;
}

void ClientManager::handleContactsResponsePackage(const Package& package)
{
    std::vector<QVariant> data;
    QVariant packageData {package.data()};
    QSequentialIterable contactsData = packageData.value<QSequentialIterable>();

    for (const auto& entry : contactsData) {
        data.push_back(std::move(entry));
    }

    emit contactsResponse(transform(data));
}
