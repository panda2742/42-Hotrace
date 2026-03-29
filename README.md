# 42-Hotrace

Fast and efficient research!

| Project | 42 Rushes - Hotrace        |
|:--------|---------------------------:|
| Locked  | 2026-02-27 20:42 CET:+0100 |
| Closed  | 2026-03-01 23:42 CET:+0100 |
| Grade   | 97%  (1⭐ outstanding)    |
| Retries | 0                          |

---

# 1. Program Overview

Hotrace is a key-value lookup engine that reads from `stdin` in two distinct phases separated by a single blank line.

|Phase|Behaviour|
|-|-|
|Phase 0|Before the blank line - Pairs of consecutive lines define entries: odd lines are keys, even lines are values. They are inserted into an in-memory hash map.|
|Phase 1|After blank line - Each line is treated as a query key. If the key exists the associated value is printed; otherwise an error message is written.|

# 2. Data Structures

## 2.1 t_slot - One hash table bucket
```c
typedef struct s_slot
{
	char		*key;	// NULL = empty slot
						// (char *)1 = tombstone (deleted)
						// anything else = live entry
	char		*value;	// heap-allocated value string
	uint32_t	hash;	// pre-computed FNV-1a hash of key
}				t_slot;
```

## 2.2 t_hm - The hash map
```c
typedef struct s_hm
{
	t_slot	*slots;		// contiguous array of t_slot
	t_size	capacity;	// total number of slots (always a power of 2)
	t_size	mask;		// capacity - 1 -> used for fast modulo
	t_size	count;		// number of live entries
	t_size	dead;		// number of tombstone slots
}			t_hm;
```

Because capacity is always a power of two, the modulo operation idx % capacity becomes idx & mask, which is a single AND instruction.

# 3. Hash Map - Algorithm

## 3.1 Design: open addressing with linear probing
The map uses open addressing: all slots live in one flat array. When a slot is occupied, the algorithm probes linearly (idx+1, idx+2,...) until it finds either an empty slot or the target key.
This is cache-friendly because consecutives slots reside in the same cache line.

## 3.2 Tombstones
Deletion is not implemented in the provided files, but the design accounts for it. A deleted slot is marked with the sentinel pointer (char*)1 instead of NULL. This distinction is critical:

|Slot state|Meaning during probe|
|-|-|
|key == NULL|Empty - stop probing, key not found|
|key == (char *)1|Tombstone - skip, but keep probing|
|key == valid ptr|Live entry - compare hashes then strings|

## 3.3 hm_insert - step by step
```
1.	Compute load = (count + dead + 1).  If load * DEN > cap * NUM → resize.
2.	h   = hash_fnv1a(key)
3.	idx = h & mask	// start position
4.	Loop:
	a. slot is NULL  →  write (key, value, h) here (or at first tombstone
						seen along the way).  count++.  Return.
	b. slot matches (same hash AND simd_strcmp)  →  update value (see fix).
	c. slot is tombstone  →  record position, keep probing.
	d. otherwise  →  idx = (idx+1) & mask, continue.
```

## 3.4 hm_get - lookup
Identical probe sequence but read-only. Returns NULL only when a truly empty slot (`key == NULL`) is encountered; tombstones are skipped. This guarantees correctness even after deletions.

## 3.5 hm_resize - growth
When the load threshold is exceeded the table doubles in capacity. All live entries (skipping tombstones) are re-inserted into the new array. The dead counter resets to zero because all tombstones are discarded. The mask updates to new_capacity - 1.

## 3.6 Load factor
Controlled by HM_LOAD_NUM and HM_LOAD_DEN (e.g. 3/4 = 75%). Resize triggers before inserting when:
```
(count + dead + 1) * HM_LOAD_DEN  >  capacity * HM_LOAD_NUM
```
Using integer arithmetic avoids any floating-point overhead.

## 3.7 FNV-1a hash (hash_fnv1a)
Not included in the uploaded sources but referenced everywhere. FNV-1a iterates over every byte of the string:
```
hash = FNV_OFFSET_BASIS   // 2166136261 for 32-bit
for each byte b:
    hash ^= b
    hash *= FNV_PRIME     // 16777619 for 32-bit
```
XOR-then-multiply is fast and provides good avalanche for typical short ASCII strings like configuration keys.

# 4. main.c - Program flow

## 4.1 run_()
The central dispatch loop reads lines one by one. It tracks the current phase (mode 0 = populate, mode 1 = query). A blank line (length == 1, i.e. just the newline character) acts as the phase separator.
```
mode = 0
while line != NULL:
    if len(line) == 1 (blank line):
        if mode == 0: free line, set mode = 1, advance
        (if mode == 1: falls through to dispatch1 which handles it gracefully)
    if mode == 0: dispatch0(hm, &tmp, &line)
    if mode == 1: dispatch1(hm, &line)
```

## 4.2 dispatch0_  –  Building the map
Uses a tmp pointer to pair consecutive lines. On odd lines (`tmp==NULL`) the line becomes the key candidate stored in tmp. On even lines, both tmp and line have their trailing newline stripped (\0-terminated), and `hm_insert(hm, tmp, line)` is called. tmp resets to NULL.

## 4.3 dispatch1_  –  Querying
The trailing newline is stripped from line. hm_get looks up the key. The result (or "key: not found") is written with two `write()` calls to avoid string concatenation allocations.

# 5. Assembly Deep Dives

## 5.1 ft_sstrlen - SWAR null-byte detection
`ft_sstrlen` is a fast `strlen` replacement. It first aligns the pointer to an 8-byte boundary byte-by-byte, then reads 8 bytes at a time using the SWAR trick (SIMD Within A Register).

### The SWAR null-byte formula
```
mask =	(word - 0x0101010101010101UL)
		& ~word
		& 0x8080808080808080UL
```
Step-by-step for a single byte b inside the 64-bit word:

|Expression|What it does|
|-|-|
|`b - 0x01`|If `b==0: 0x00-0x01` underflows to 0xFF (sets bit 7). If `b>=1`: `result < 0x80`.|
|`& ~b`|Clears false positives where b was already `≥ 0x80` (high bit set before subtraction).|
|`& 0x80`|Isolates only bit 7 of each byte - the 'null detector'
flag.|

### BSF instruction (Bit Scan Forward)
```
bsfq %1, %0		// 64-bit version
				// Scans from bit 0 upward.
				// Writes index of FIRST set bit into destination.
				// Undefined if source == 0 (always guard with if mask != 0).
```
The result is a bit position (0–63). Dividing by 8 (>> 3) converts it to a byte offset within the 8-byte word. The final length is then:
```
len = (ptr_after_word - str) - 8 + (bit_pos >> 3)
```
The -8 accounts for the fact that ptr was already incremented past the word before the null check.

## 6.2 ft_memmove - Two-direction copy
The direction of copying must match the overlap situation. The C code dispatches to one of two assembly routines:
|Condition|Routine|Direction|
|-|-|-|
|`dst < src`|`ft_memmove_asm1_`|Forward  (low→high address)|
|`dst > src`|`ft_memmove_asm2_`|Backward (high→low address)|

### Forward copy (ft_memmove_asm1_) - annotated
```asm
xorq		%%rcx, %%rcx		// rcx = 0  (byte offset into src/dst)

1:	cmpq	$8, %2				// remaining >= 8?
	jl		2f					// no → go to byte loop
	movq	(%1, %%rcx), %%rax	// load 8 bytes from src+rcx
	movq	%%rax, (%0, %%rcx)	// store 8 bytes to   dst+rcx
	addq	$8, %%rcx			// offset  += 8
	subq	$8, %2				// remaining -= 8
	jmp 	1b

2:	testq	%2, %2				// remaining == 0?
	je		3f
	movb	(%1, %%rcx), %%al	// load 1 byte
	movb	%%al, (%0, %%rcx)	// store 1 byte
	incq	%%rcx
	decq	%2
	jmp		2b
3:
```
Uses `rcx` as a shared offset for both src and dst, so only one counter is needed. The inner 8-byte loop copies 64 bits per iteration; the remainder loop handles 0–7 trailing bytes.
The operand constraint '+r' on dst, src, len means the compiler allocates registers for them and the assembly is allowed to modify them - no stack spill needed.

### Backward copy (ft_memmove_asm2_) - annotated
```asm
movq		%2, %%rcx			// rcx = len  (start at the END)

1:	cmpq	$8, %%rcx
	jl		2f
	subq	$8, %%rcx			// move cursor BACKWARDS by 8
	movq	(%1, %%rcx), %%rax	// load from src+rcx (high address first)
	movq	%%rax, (%0, %%rcx)	// store to  dst+rcx
	jmp		1b

2:	testq	%%rcx, %%rcx
	je		3f
	decq	%%rcx				// move cursor back by 1
	movb	(%1, %%rcx), %%al
	movb	%%al, (%0, %%rcx)
	jmp		2b
3:
```
Decrementing before the load means the final iteration copies byte at offset 0. The high-to-low order is essential when dst > src and regions overlap: writing low addresses first would corrupt bytes that haven't been read yet.

## 6.3 simd_strcmp - SSE2 16-byte comparison
Compares two strings 16 bytes at a time using 128-bit SSE2 registers. Handles strings of any length by checking for the null terminator within each chunk.

### Page-boundary guard
```
if (((uintptr_t)a & 0xFFF) > 4080 || ((uintptr_t)b & 0xFFF) > 4080)
	// fall back to scalar byte-by-byte
```
Loading 16 bytes from a pointer close to a page boundary (offset > 4080 = 0xFF0, so < 16 bytes remain in the page) would cause the unaligned movdqu to read into the next page, which might not be mapped. The scalar fallback is safe.

### Core SSE2 block - annotated
```asm
movdqu  (%2), %%xmm0		// xmm0 = 16 bytes of string a (unaligned load)
movdqu  (%3), %%xmm1		// xmm1 = 16 bytes of string b
pxor    %%xmm2, %%xmm		// xmm2 = 0x00...00  (zero register)

pcmpeqb %%xmm2, %%xmm0		// xmm0[i] = 0xFF if a[i]==0x00, else 0x00
							// → null-byte detector for string a
pmovmskb %%xmm0, %0			// *end = 16-bit mask: bit i set ↔ a[i] was null

movdqu  (%2), %%xmm0		// reload original a
pcmpeqb %%xmm1, %%xmm0		// xmm0[i] = 0xFF if a[i]==b[i], else 0x00
pmovmskb %%xmm0, %1			// *eq  = 16-bit mask: bit i set ↔ a[i]==b[i]
```

### PMOVMSKB - extracting the comparison result
PMOVMSKB extracts the most-significant bit (bit 7) of each of the 16 bytes in an XMM register and packs them into a 16-bit integer. This is the key instruction that converts a vector comparison into a scalar bitmask.
```
xmm = [ 0xFF, 0x00, 0xFF, 0xFF, 0x00, ... ]
		 ↑                  ↑
		bit7=1              bit7=0

pmovmskb → 0b 0...0 1 0 1 1 0 ...
					↑ ↑ ↑ ↑ ↑
				  bit index 0→15
```

### Back in C: masking to the relevant bytes
```
bsfl %1, %0			// BSF 32-bit: find first set bit in *end
					// null_pos = position of first null byte in a

relevant = (1 << (null_pos + 1)) - 1
			// mask of bits 0..null_pos  (all bytes up to and incl. null)

return ((eq & relevant) == relevant)
		// true iff ALL bytes up to the null terminator were equal
```
Example: if `a = "hi\0..."` then `null_pos=2`, `relevant=0b0000_0111`, and we check that bits 0,1,2 of eq are all set (i.e. `a[0]==b[0]`, `a[1]==b[1]`, `a[2]==b[2]`).
If no null was found in the 16 bytes (`end==0`) and all 16 bytes matched (`eq==0xFFFF`), advance both pointers by 16 and repeat.

## 6.4 next_pow2 - BSR trick
```asm
bsrq %1, %0		// Bit Scan Reverse (64-bit)
				// Scans from bit 63 downward.
				// Writes index of HIGHEST set bit into destination.
```
After `n--` (to handle the case where n is already a power of two - though that's returned early), BSR gives the position of the leading bit. Shifting 1 left by (highest_bit + 1) produces the next power of two.
```
n = 5	→	n-- = 4 = 0b100	→	BSR = 2	→	1 << 3 = 8  ✓
n = 7	→	n-- = 6 = 0b110	→	BSR = 2	→	1 << 3 = 8  ✓
n = 8	→	already power of 2, returned early as 8  ✓
```
