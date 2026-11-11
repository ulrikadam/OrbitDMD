// Remplacement minimal de <circle/types.h> pour compiler les tests sous Windows
#ifndef _circle_types_h
#define _circle_types_h

#include <stddef.h>
#include <stdint.h>

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;
typedef int		boolean;
typedef uintptr_t	uintptr;

#define FALSE	0
#define TRUE	1

#endif
