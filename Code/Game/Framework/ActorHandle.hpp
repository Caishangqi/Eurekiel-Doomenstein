﻿#pragma once

struct ActorHandle
{
public:
    ActorHandle();
    ActorHandle(unsigned int uid, unsigned int index);

    bool         IsValid() const;
    unsigned int GetIndex() const;
    bool         operator==(const ActorHandle& other) const;
    bool         operator!=(const ActorHandle& other) const;

    static const ActorHandle INVALID;

private:
    unsigned int m_data;
};
