#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper: check if a string is valid binary (only '0' and '1')
int is_binary(const char *s) {
    for (int i = 0; s[i]; i++)
        if (s[i] != '0' && s[i] != '1') return 0;
    return 1;
}

int main(int argc, char *argv[]) {
    // ── Step 0: Check arguments ──────────────────────────────────
    // Usage:  ./hamming encode <4-bit data>
    //         ./hamming correct <7-bit received>
    if (argc != 3) {
        printf("Usage:\n");
        printf("  %s encode  <4-bit data>      e.g. 1011\n", argv[0]);
        printf("  %s correct <7-bit received>  e.g. 0110001\n", argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    const char *input = argv[2];

    // ── ENCODE MODE ──────────────────────────────────────────────
    if (strcmp(mode, "encode") == 0) {

        // Step 1: Validate input is exactly 4 binary bits
        if (strlen(input) != 4 || !is_binary(input)) {
            printf("Error: provide exactly 4 binary bits (e.g. 1011)\n");
            return 1;
        }

        // Step 2: Place data bits into slots 3, 5, 6, 7
        //         Parity bits P1, P2, P4 go in slots 1, 2, 4
        int bits[8]; // index 0 unused; we use 1..7
        bits[3] = input[0] - '0';
        bits[5] = input[1] - '0';
        bits[6] = input[2] - '0';
        bits[7] = input[3] - '0';

        // Step 3: Compute each parity bit (even parity)
        // P1 covers slots 1,3,5,7 → only data slots 3,5,7 matter
        int ones_p1 = bits[3] + bits[5] + bits[7];
        bits[1] = (ones_p1 % 2) != 0; // flip to make total even

        // P2 covers slots 2,3,6,7 → only data slots 3,6,7 matter
        int ones_p2 = bits[3] + bits[6] + bits[7];
        bits[2] = (ones_p2 % 2) != 0;

        // P4 covers slots 4,5,6,7 → only data slots 5,6,7 matter
        int ones_p4 = bits[5] + bits[6] + bits[7];
        bits[4] = (ones_p4 % 2) != 0;

        // Step 4: Print the 7-bit encoded codeword
        printf("Data:   %s\n", input);
        printf("Encoded: ");
        for (int i = 1; i <= 7; i++) printf("%d", bits[i]);
        printf("\n");
        printf("Slots:  1 2 3 4 5 6 7\n");
        printf("        P1 P2 D1 P4 D2 D3 D4\n");

    // ── CORRECT MODE ─────────────────────────────────────────────
    } else if (strcmp(mode, "correct") == 0) {

        // Step 1: Validate input is exactly 7 binary bits
        if (strlen(input) != 7 || !is_binary(input)) {
            printf("Error: provide exactly 7 binary bits (e.g. 0110011)\n");
            return 1;
        }

        // Step 2: Load received bits into array (slots 1..7)
        int bits[8];
        for (int i = 1; i <= 7; i++)
            bits[i] = input[i - 1] - '0';

        // Step 3: Recalculate syndrome (which parity checks fail)
        // P1 check: XOR of slots 1,3,5,7
        int p1 = bits[1] ^ bits[3] ^ bits[5] ^ bits[7];
        // P2 check: XOR of slots 2,3,6,7
        int p2 = bits[2] ^ bits[3] ^ bits[6] ^ bits[7];
        // P4 check: XOR of slots 4,5,6,7
        int p4 = bits[4] ^ bits[5] ^ bits[6] ^ bits[7];

        // Step 4: Syndrome = P4*4 + P2*2 + P1*1 → gives error position
        int syndrome = p4 * 4 + p2 * 2 + p1;

        printf("Received: ");
        for (int i = 1; i <= 7; i++) printf("%d", bits[i]);
        printf("\n");
        printf("Syndrome: P4=%d P2=%d P1=%d → position %d\n", p4, p2, p1, syndrome);

        if (syndrome == 0) {
            // Step 5a: No error detected
            printf("No error detected. Data: %d%d%d%d\n",
                   bits[3], bits[5], bits[6], bits[7]);
        } else if (syndrome >= 1 && syndrome <= 7) {
            // Step 5b: Flip the bit at the syndrome position
            printf("Error at slot %d! Flipping it...\n", syndrome);
            bits[syndrome] ^= 1; // invert the bit

            printf("Corrected: ");
            for (int i = 1; i <= 7; i++) printf("%d", bits[i]);
            printf("\n");
            printf("Data: %d%d%d%d\n", bits[3], bits[5], bits[6], bits[7]);
        }
    } else {
        printf("Unknown mode '%s'. Use 'encode' or 'correct'.\n", mode);
        return 1;
    }

    return 0;
}   