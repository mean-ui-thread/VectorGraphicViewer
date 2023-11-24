#ifndef BIT_MASK_H
#define BIT_MASK_H

template <typename E>
class BitMask
{
    typedef typename std::underlying_type<E>::type EType;
    EType value;
public:
    BitMask() :
        value(0)
    {
    }

    BitMask(E e) :
        value(static_cast<EType>(e))
    {
    }

    BitMask(const BitMask &) = default;
    BitMask &operator=(const BitMask &) = default;

    void operator=(E e)
    {
        value = static_cast<EType>(e);
    }

    void set(E e)
    {
        value |= static_cast<EType>(e);
    }

    void reset(E e)
    {
        value &= ~static_cast<EType>(e);
    }

    bool test(E e) const
    {
        return value & static_cast<EType>(e);
    }

    operator EType() const { return value; }
};

#endif // BIT_MASK_H