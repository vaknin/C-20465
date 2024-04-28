/*
 * Program Name: Partial Sums Calculator
 * Description:
 * This program reads a series of integers from the user, calculates the partial sums of these integers,
 * and displays the results. The integers are expected to be entered in a single line, separated by spaces.
 * The program demonstrates the use of dynamic memory management and string parsing to handle user input.
 *
 * Inputs:
 * A line of integers entered by the user.
 *
 * Outputs:
 * The partial sums of the input integers.
 */

#include <stdio.h>
#include <stdlib.h>

/* Function: partialSums
 * Description:
 * Calculates the partial sums of an array of integers.
 * It dynamically allocates an array to store the partial sums and returns a pointer to this array.
 *
 * Parameters:
 * arr - an array of integers
 * size - the number of elements in the array
 *
 * Returns:
 * A pointer to an array containing the partial sums of the input array.
 */
int* partialSums(int arr[], int size) {
    int partial_sum = 0;
    int i;
    int* psums = malloc(size * sizeof(int)); /* Allocate memory for the partial sums */

    if (psums == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < size; i++) {
        partial_sum += arr[i];
        psums[i] = partial_sum; /* Store each partial sum */
    }

    return psums; /* Return the array of partial sums */
}

/* Function: main
 * Description:
 * The main function handles user input, calls the partialSums function to compute partial sums,
 * and displays the results. It reads a single line of input from the user, parses the integers,
 * and manages memory used for the computations.
 *
 * Outputs:
 * Prints the computed partial sums to the standard output.
 */
int main() {
    int array[1024]; /* Array to store user input */
    int count = 0;
    int* psums;
    int i, num, nchars;
    char line[1024]; /* Buffer to hold input line */

    printf("Enter numbers separated by spaces (e.g., '1 2 3 4 5'):\n");
    if (fgets(line, sizeof(line), stdin)) {
        char *p = line;
        /* Parse integers from the line */
        while (sscanf(p, "%d%n", &num, &nchars) == 1) {
            array[count++] = num;
            p += nchars; /* Move pointer past the last integer read */
            if (count >= 1024) break; /* Prevent overflow if input is too large */
        }
    }

    psums = partialSums(array, count); /* Compute partial sums */
    for (i = 0; i < count; i++) {
        printf("new num is: %d\n", psums[i]); /* Display each partial sum */
    }

    free(psums); /* Free allocated memory */

    return 0;
}