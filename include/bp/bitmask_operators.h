#ifndef BP_BITMASK_OPERATORS_H
#define BP_BITMASK_OPERATORS_H

#define BP_DECLARE_BITMASK_OPERATORS(enum_name) \
	enum_name operator|(enum_name a, enum_name b); \
	enum_name operator&(enum_name a, enum_name b); \
	enum_name operator^(enum_name a, enum_name b); \
	enum_name operator~(enum_name a); \
	enum_name operator|=(enum_name& a, enum_name b); \
	enum_name operator&=(enum_name& a, enum_name b); \
	enum_name operator^=(enum_name& a, enum_name b);

#define BP_DEFINE_BITMASK_OPERATORS(enum_name) \
	enum_name operator|(enum_name a, enum_name b) { \
		return (enum_name) ((int) a | (int) b); \
	} \
	enum_name operator&(enum_name a, enum_name b) { \
		return (enum_name) ((int) a & (int) b); \
	} \
	enum_name operator^(enum_name a, enum_name b) { \
		return (enum_name) ((int) a ^ (int) b); \
	} \
	enum_name operator~(enum_name a) { \
		return (enum_name) ~((int) a); \
	} \
	enum_name operator|=(enum_name& a, enum_name b) { \
		return (enum_name) (((int&) a) |= (int) b); \
	} \
	enum_name operator&=(enum_name& a, enum_name b) { \
		return (enum_name) (((int&) a) &= (int) b); \
	} \
	enum_name operator^=(enum_name& a, enum_name b) { \
		return (enum_name) (((int&) a) ^= (int) b); \
	}

#endif
