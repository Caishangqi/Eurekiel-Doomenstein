#pragma once
#include <string>

struct ActorHandle
{
    ActorHandle();
    ActorHandle(unsigned int data);
    ActorHandle(std::string data);
    ActorHandle(unsigned int uid, unsigned int index);

    bool         IsValid() const;
    unsigned int GetIndex() const;
    unsigned int GetData();
    std::string  ToString() const;
    bool         operator==(const ActorHandle& other) const;
    bool         operator!=(const ActorHandle& other) const;

    static const ActorHandle INVALID;

private:
    unsigned int m_data;
};
