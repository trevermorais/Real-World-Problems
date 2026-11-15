# Hamming Code — Single Bit Error Correction

## Problem
A single bit in a transmitted message can flip (1→0 or 0→1). Hamming code adds redundant **parity bits** so the receiver can **detect and correct** a single-bit error without retransmission.

## Encoding (Sender Side)

### Step 1: Determine parity bits needed
For **d** data bits, find **r** parity bits where `2^r ≥ d + r + 1`.
- 4 data bits → 3 parity bits (P1, P2, P4) → 7-bit codeword.

### Step 2: Assign slots
Parity bits go in **power-of-2** slots (1, 2, 4, 8…); data fills the rest.

| Slot | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|------|---|---|---|---|---|---|---|
| Role | P1 | P2 | D1 | P4 | D2 | D3 | D4 |

### Step 3: Each parity bit's coverage
A parity bit covers slots whose binary index has a 1 in that bit's position:

| Parity | Covers slots |
|--------|-------------|
| P1 (bit 0) | 1, 3, 5, 7 |
| P2 (bit 1) | 2, 3, 6, 7 |
| P4 (bit 2) | 4, 5, 6, 7 |

### Step 4: Set parity (even parity)
Count 1s in the **data** slots of each group. If odd → set parity to 1; if even → set to 0.

**Example:** data = `1011`

| Parity | Data slots | 1s count | Parity bit |
|--------|-----------|----------|------------|
| P1 | 3,5,7 → 1,0,1 | 2 (even) | **0** |
| P2 | 3,6,7 → 1,1,1 | 3 (odd) | **1** |
| P4 | 5,6,7 → 0,1,1 | 2 (even) | **0** |

**Encoded output:** `0 1 1 0 0 1 1`

## Correction (Receiver Side)

### Step 1: Recalculate syndrome
XOR each parity group. A result of 1 means that group's "even" promise is broken.

### Step 2: Read syndrome as a binary number
Syndrome = `P4 P2 P1` → gives the **slot number** of the error.

| Syndrome | Meaning |
|:--------:|---------|
| 000 | No error |
| 001 | Slot 1 |
| 010 | Slot 2 |
| 011 | Slot 3 |
| 100 | Slot 4 |
| 101 | Slot 5 |
| 110 | Slot 6 |
| 111 | Slot 7 |

### Step 3: Flip the bit at that slot

**Example:** slot 7 flipped → all three fail → syndrome `111` = 7 → flip slot 7.

## Why It Works
Each slot has a **unique membership pattern** across the three groups. A single flip breaks exactly the groups it belongs to, and no two slots share the same pattern — so the failure combination is a **unique fingerprint** of the position.

## Limitation
Corrects **at most 1** error. Two simultaneous flips can produce a valid (but wrong) syndrome. Real systems (e.g., ECC RAM) add an overall parity bit for **double-error detection** (SECDED).

## C Program Usage
```bash
gcc -o hamming hamming.c
./hamming encode 1011      # → 0110011
./hamming correct 0110001  # → error at slot 6, corrected   