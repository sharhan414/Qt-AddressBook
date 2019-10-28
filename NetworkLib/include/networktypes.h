#pragma once

namespace net
{
enum class PackageType {
    INVALID = 10,
    CONTACTS_REQUEST,
    CONTACTS_RESPONSE
};

enum class ConnectionState {
    Connected,
    Disconnected,
    Connecting
};
}