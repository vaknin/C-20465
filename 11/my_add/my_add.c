/*
 * Program Name: Binary Addition Simulator
 * Description:
 * This program simulates the addition of two unsigned integers in binary form without using the '+' operator.
 * It iterates bit by bit, from the least significant bit to the most significant bit, to calculate the sum and manage carries.
 * The program demonstrates fundamental bitwise operations and binary arithmetic principles.
 *
 * Algorithms:
 * The addition is performed using bitwise XOR for sum calculation, bitwise AND for carry determination,
 * and bitwise shifts to move through each bit position. A carry is propagated forward through the loop until no carry remains.
 *
 * Inputs:
 * Two unsigned integers entered by the user or redirected from a file.
 *
 * Outputs:
 * - The binary representation of each input number.
 * - The binary representation and base 10 value of the sum of the two numbers.
 */

#include <stdio.h>

/*
 * Function: print_binary
 * Description:
 * Converts an unsigned integer to its binary representation and prints it.
 * The function iterates through each bit of the input number, starting from the most significant bit,
 * and prints '1' or '0' depending on whether the bit is set.
 *
 * Parameters:
 * num - The unsigned integer to be converted and printed.
 *
 * Algorithm:
 * Uses a mask initialized to the highest bit position and shifts it right through all bits of the input,
 * using bitwise AND to check if each bit is set.
 */
void print_binary(unsigned int num) {
    int bits = sizeof(num) * 8; /* Determine the number of bits in 'num'. */
    unsigned int mask = 1U << (bits - 1); /* Initialize mask to the highest bit position. */
    int i;
    for (i = 0; i < bits; i++) {
        printf("%c", (num & mask) ? '1' : '0'); /* Print '1' if the current bit is set; otherwise, print '0'. */
        mask >>= 1; /* Move the mask one bit to the right. */
    }
    printf("\n");
}

/*
 * Function: my_add
 * Description:
 * Performs binary addition of two unsigned integers without using the '+' operator.
 * It calculates the sum bit-by-bit and handles carries to accurately add the two inputs.
 *
 * Parameters:
 * a - The first unsigned integer to add.
 * b - The second unsigned integer to add.
 *
 * Algorithm:
 * Iterates through each bit position. Uses XOR to calculate the sum of two bits, AND to find the carry,
 * and a shift operation to prepare for the next bit's addition. This process repeats for all bits of the inputs.
 *
 * Returns:
 * The sum of 'a' and 'b' as an unsigned integer.
 */
unsigned int my_add(unsigned int a, unsigned int b) {
    unsigned int sum = 0, carry = 0;
    unsigned int shift = 1;
    int i;
    for (i = 0; i < sizeof(a) * 8; i++) {
        /* Isolate the current bit for both 'a' and 'b' */
        unsigned int bit_a = a & shift;
        unsigned int bit_b = b & shift;
        /* Calculate the sum of the current bits, including the carry */
        unsigned int sum_bit = (bit_a ^ bit_b) ^ carry;
        /* Set the corresponding bit in 'sum' */
        sum |= (sum_bit & shift);

        /* Determine the carry for the next bit */
        carry = (bit_a & bit_b) | (bit_a & carry) | (bit_b & carry);
        carry <<= 1; /* Prepare the carry for the next higher bit */
        shift <<= 1; /* Shift to the next bit position */
    }

    /* Print binary representations of the operands and their sum */
    printf("a in binary: ");
    print_binary(a);
    printf("b in binary: ");
    print_binary(b);
    printf("Sum in binary: ");
    print_binary(sum);

    return sum;
}

/*
 * Function: main
 * Description:
 * Collects two unsigned integers from the user or file input, performs binary addition using 'my_add',
 * and prints both the input numbers and their sum in base 10 and binary form.
 *
 * Inputs:
 * Two unsigned integers provided by the user or redirected from a file.
 *
 * Outputs:
 * The inputs and their sum, displayed in both base 10 and binary form.
 */
int main() {
    unsigned int a, b, sum;
    printf("Enter two numbers (separated by space): ");
    scanf("%u %u", &a, &b);

    /* Echo the inputs in base 10 */
    printf("Inputs: a = %u, b = %u\n", a, b);

    /* Perform binary addition and store the result */
    sum = my_add(a, b);
    /* Print the sum in base 10 */
    printf("Sum in base 10: %u\n", sum);
    return 0;
}
