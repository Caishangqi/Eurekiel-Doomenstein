#include "ActorHandle.hpp"

const ActorHandle ActorHandle::INVALID = ActorHandle();

ActorHandle::ActorHandle(): m_data(0)
{
}

ActorHandle::ActorHandle(unsigned int uid, unsigned int index)
{
    unsigned int salt16  = (uid & 0xFFFFU);
    unsigned int index16 = (index & 0xFFFFU);
    m_data               = (salt16 << 16) | index16;
}

bool ActorHandle::IsValid() const
{
    return this != &INVALID;
}

unsigned int ActorHandle::GetIndex() const
{
    /*
    *  Assume that the binary of m_data is  1011 0001 0010 1111 1110 0111 0000 1001 (32 bits)
    *  The binary of 0xFFFF is              0000 0000 0000 0000 1111 1111 1111 1111 (32 bits)
    *  After bitwise AND, only the lower 16 bits of m_data are retained:
    *                                       0000 0000 0000 0000 1110 0111 0000 1001
    */
    return m_data & 0xFFFF;
}

bool ActorHandle::operator==(const ActorHandle& other) const
{
    return m_data == other.m_data;
}

bool ActorHandle::operator!=(const ActorHandle& other) const
{
    return m_data != other.m_data;
}
