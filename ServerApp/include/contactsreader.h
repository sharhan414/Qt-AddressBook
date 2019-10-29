#pragma once
#include <vector>
#include <memory>
#include <QVariant>

namespace db
{
class Processor;
}
class ContactsReader
{
public:
    ContactsReader();
    std::pair<bool,
              std::vector<QVariant>> requestContactsBrowse();
    ~ContactsReader();

private:
    std::unique_ptr<db::Processor> m_dbProcessor;
};

