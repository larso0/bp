#ifndef BP_FLAGSET_H
#define BP_FLAGSET_H

#include <bitset>

namespace bp
{

/*
 * Use enum values to set bits in a std::bitset.
 * The enum values in the enum (T) can not have assigned values.
 * If enum class is used, the type must be an integer type.
 * The last element of the enum must be BP_FLAGSET_LAST.
 *
 * Examples:
 *
 * enum Flags { A, B, C, BP_FLAGSET_LAST };
 *
 * bp::FlagSet<Flags> flags;
 * //Set flags with operator <<
 * flags << A << C;
 *
 * //Use & operator to check a single flag
 * if (flags & A) { ... }
 *
 * //Use operator << on rvalue references
 * void foo(const bp::FlagSet<Flags>& flags) { ... }
 * foo(bp::FlagSet<Flags>() << A << B);
 */

template <typename T>
using FlagSet = std::bitset<static_cast<size_t>(T::BP_FLAGSET_LAST)>;

}

template <typename T>
bp::FlagSet<T>& operator<<(bp::FlagSet<T>& flags, T flag)
{
	flags[static_cast<size_t>(flag)] = true;
	return flags;
}

template <typename T>
bp::FlagSet<T>& operator<<(bp::FlagSet<T>&& flags, T flag)
{
	flags[static_cast<size_t>(flag)] = true;
	return flags;
}

template <typename T>
bp::FlagSet<T>& operator>>(bp::FlagSet<T>& flags, T flag)
{
	flags[static_cast<size_t>(flag)] = false;
	return flags;
}

template <typename T>
bp::FlagSet<T>& operator>>(bp::FlagSet<T>&& flags, T flag)
{
	flags[static_cast<size_t>(flag)] = false;
	return flags;
}

template <typename T>
bp::FlagSet<T>& operator|=(bp::FlagSet<T>& flags, T flag)
{
	flags[static_cast<size_t>(flag)] = true;
	return flags;
}

template <typename T>
bp::FlagSet<T>& operator|=(bp::FlagSet<T>&& flags, T flag)
{
	flags[static_cast<size_t>(flag)] = true;
	return flags;
}

template <typename T>
bp::FlagSet<T>& operator^=(bp::FlagSet<T>& flags, T flag)
{
	flags.flip(static_cast<size_t>(flag));
	return flags;
}

template <typename T>
bp::FlagSet<T>& operator^=(bp::FlagSet<T>&& flags, T flag)
{
	flags.flip(static_cast<size_t>(flag));
	return flags;
}

template <typename T>
bool operator&(const bp::FlagSet<T>& flags, T flag) { return flags[static_cast<size_t>(flag)]; }

#endif