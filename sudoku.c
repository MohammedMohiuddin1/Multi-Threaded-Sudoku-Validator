
/*
 CSS 430 Program 2: Multi-Threaded Sudoku Validator
 Mohammed Mohiuddin
 compile: gcc -o sudoku -lm -pthread sudoku.c
 run: ./sudoku <puzzle_name>.txt
 where <puzzle_name> is replaced with the actual puzzle name

*/


#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//parameters struct holds the data for all the threads that are to be generated
typedef struct {

    //index for row
    int row;
    //index for column 
    int col;
    //2D grid to keep track of grids 
    int** grid;
    //overall length of puzzle
    int length;
    //array to keep track of valid indexes
    int* isValid; 
} parameters;

//global variable to keep track of grids
int num_of_grids = 1;

//Method Declaration 
void* checkIsValidRow(void* thread_data); //method to be used by threads to validate rows
void* checkIsValidColumn(void* thread_data); //method to be used by threads to validate columns
void* checkIsValidGrid(void* thread_data); //method to be used by threads to validate grids
bool isPuzzleComplete(int** puzzle, int length); //method to ensure that the puzzle is complete. (Not having 0s) 
void* fillZeros(void* thread_data); //fillZeroes extra credit method to fill in missing zeroes. 


// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {

    //check if the puzzle is complete 
    *complete = isPuzzleComplete(grid, psize);

    //if its not then instantly set to false and return.
    if (*complete == false) 
    { 
        return; 
    }

    //Allocate memory for validation of rows where 0 is valid and 1 is invalid 
    int* row_validation = malloc((psize + 1) * sizeof(*row_validation));

    //Allocate memory for validation of columns where 0 is valid and 1 is invalid
    int* col_validation = malloc((psize + 1) * sizeof(*col_validation));

    //Allocate memory for validation of grids where 0 is valid and 1 is invalid
    int* grid_validation = malloc((psize + 1) * sizeof(*grid_validation));

    //initialize all values of arrays to -1; 
    for (int i = 1; i <= psize; i++) 
    {
        row_validation[i] = -1;
        col_validation[i] = -1;
        grid_validation[i] = -1;
    }

    //allocate memory for row, column, and grid threads. 
    pthread_t* row_threads = malloc(sizeof(pthread_t) * (psize + 1));
    pthread_t* col_threads = malloc(sizeof(pthread_t) * (psize + 1));
    pthread_t* grid_threads = malloc(sizeof(pthread_t) * (psize + 1));
    
    //Error handling to ensure that the memory was properly allocated for all threads
    if (row_threads == NULL) 
    {
        printf("Failed to allocate memory for row threads \n");
        exit(EXIT_FAILURE);
    }

    if (col_threads == NULL) 
    {
        printf("Failed to allocate memory for column threads \n");
        exit(EXIT_FAILURE);
    }

    if (grid_threads == NULL) 
    {
        printf("Failed to allocate memory for grid threads \n");
        exit(EXIT_FAILURE);
    }

    //loop to create threads for rows.
    for (int row = 1; row <= psize; row++) 
    {
        //create struct to assign appropriate data to parameters 
        parameters* thread = (parameters*) malloc(sizeof(parameters));
        thread->row = row;
        thread->col = -1;
        thread->grid = grid;
        thread->length = psize;
        thread->isValid = row_validation;

        //create the row threads, NOTE: these rowthreads will now go and execute the helper function checkIsValidRow() 
        if (pthread_create(&row_threads[row], NULL, checkIsValidRow, (void*) thread)) {
            printf("Failed to create row threads");
            exit(EXIT_FAILURE);
        }
    }

    //loop to create threads for columns
    for (int col = 1; col <= psize; col++) 
    {
        //create struct to assign appropriate data to parameters 
        parameters* thread = (parameters*) malloc(sizeof(parameters));
        thread->row = -1;
        thread->col = col;
        thread->grid = grid;
        thread->length = psize;
        thread->isValid = col_validation;

        //create the column threads, NOTE: these columnthreads will now go and execute the helper function checkIsValidColumn() 
        if (pthread_create(&col_threads[col], NULL, checkIsValidColumn, (void*) thread)) {
            printf("Failed to create column threads");
            exit(EXIT_FAILURE);
        }
    }

    //since the puzzle is guranteed to be a perfect square, we can easily calculate grid size by taking the square root of the size of the puzzle. 
    int grid_size = sqrt(psize);

    //index to keep track of grid threads
    int count = 1;

    //loops to create grid threads
    for (int row = 1; row <= psize; row += grid_size) 
    {
        for (int col = 1; col <= psize; col += grid_size) 
        {
            //create struct to assign appropriate data to parameters 
            parameters* thread = (parameters*) malloc(sizeof(parameters));
            thread->row = row;
            thread->col = col;
            thread->grid = grid;
            thread->length = psize;
            thread->isValid = grid_validation;

            //create the grid threads, NOTE: these grid threads will now go and execute the helper function checkIsValidGrid()
            if (pthread_create(&grid_threads[count], NULL, checkIsValidGrid, (void*) thread)) {
                printf("Failed to create grid threads");
                exit(EXIT_FAILURE);
            }
            count++;
        }
    }

    //join all threads and wait for them to complete
    for (int i = 1; i <= psize; i++) 
    {
        pthread_join(row_threads[i], NULL);
        pthread_join(col_threads[i], NULL);
        pthread_join(grid_threads[i], NULL);
    }

    //check validation of the arrays to ensure that puzzle is valid in terms of all aspects
    *valid = true;

    for (int i = 1; i <= psize; i++) 
    {
        //error handling to make sure that to ensure that all rows, grids, and columns were checked.
        if (row_validation[i] == -1) 
        {
            printf("Some rows were not checked for validity \n");
            *valid = false;
            break;
        }
        if (col_validation[i] == -1) 
        {
            printf("Some columns were not checked for validity \n");
            *valid = false;
            break;
        }
        if (grid_validation[i] == -1) 
        {
            printf("Some grids were not checked for validity \n");
            *valid = false;
            break;
        }
        if (row_validation[i] == 0 || col_validation[i] == 0 || grid_validation[i] == 0) 
        {
            *valid = false;
            break;
        }
    }
    //once we reach this point if *valid is still true then the puzzle is valid else its gonna be false. 

    //free the allocated memory
    free(row_validation);
    free(col_validation);
    free(grid_validation);

    free(row_threads);
    free(col_threads);
    free(grid_threads);

}

//method is supposed to be executed by thread to check if the puzzle's row is valid
//uses all data from the struct primarily emphasising length, grid, row, and the isValid array. 
void* checkIsValidRow(void* thread_data) {

    parameters* thread = (parameters*) thread_data; 

    //the approach is to have an array for each row that all numbers from 1 to N are present in the array. 
    //In this case a number is found if it is equal to 1 and 0 otherwise. 
    int found_values_holder[thread->length + 1];

    //first we simply set all values of the array to 0 indicating not found.
    for(int i = 1; i <= thread->length; i++) 
    {
        found_values_holder[i] = 0;
    }

    //next we loop through and set the values that are found to 1. For example, if thred->grid[thread->row][i] = 3; then the position at 3 in the found array would be set to 1
    //indicating that its found.
    for(int i = 1; i <= thread->length; i++) 
    {
        found_values_holder[thread->grid[thread->row][i]] = 1;
    }

    //once this is done
    bool isValidRow = true; 

    //we then loop through and check the array to ensure that the values in the row were all found. 
    for(int i = 1; i <= thread->length; i++) 
    {
        if(found_values_holder[i] == 0) //if any of them still aren't found then that means the row is invalid
        {
            isValidRow = false;
            break; 
        }
    }

    //remember isValid is 0 if its invalid and 1 if its valid
    if (!isValidRow) 
    {
        thread->isValid[thread->row] = 0;
    }
    else 
    {
        thread->isValid[thread->row] = 1;
    }
    
    return NULL;
}

//method is supposed to be executed by thread to check if the puzzle's column is valid
//uses all data from the struct primarily emphasising length, grid, column, and the isValid array. 
void* checkIsValidColumn(void* thread_data) {
    
    parameters* thread = (parameters*) thread_data;

    //we take the same approach as we did for the row here but for column. 
    int found_values_holder[thread->length + 1];

    //set to zero
    for(int i = 1; i <= thread->length; i++) 
    {
        found_values_holder[i] = 0;
    }

    //establish found values
    for(int i = 1; i <= thread->length; i++) 
    {
        found_values_holder[thread->grid[i][thread->col]] = 1;
    }

    bool isValidCol = true;

    //ensure all vals are found
    for(int i = 1; i <= thread->length; i++) 
    {
        if(found_values_holder[i] == 0) 
        {
            isValidCol = false;
            break;
        }
    }

    //set validity based on results
    if (!isValidCol) 
    {
        thread->isValid[thread->col] = 0;
    }
    else 
    {
        thread->isValid[thread->col] = 1;
    }

    return NULL;
}

//method is supposed to be executed by thread to check if the puzzle's grid is valid
//uses all data from the struct primarily emphasising all data members present in the struct
void* checkIsValidGrid(void* thread_data) {

    parameters* thread = (parameters*) thread_data;

    //establish sort of the same approach here with a little optimization to accomodate for the grid. 
    int found_values_holder[thread->length + 1];

    for(int i = 1; i <= thread->length; i++) 
    {
        found_values_holder[i] = 0;
    }

    //since size is guranteed to be a perfect square, sqrt would gurantee the grid size.
    int grid_size = sqrt(thread->length);

    //loop through and set each value found in the grid to 1
    for(int i = thread->row; i < thread->row + grid_size; i++) 
    {
        for(int j = thread->col; j < thread->col + grid_size; j++) 
        {
            found_values_holder[thread->grid[i][j]] = 1;
        }
    }

    bool isValidGrid = true;

    //check validity to ensure that all values are indeed found
    for(int i = 1; i <= thread->length; i++) 
    {
        if(found_values_holder[i] == 0) 
        {
            isValidGrid = false;
            break;
        }
    }

    //assign validity based on results
    if (!isValidGrid) 
    {
        thread->isValid[num_of_grids] = 0;
    }
    else 
    {
        thread->isValid[num_of_grids] = 1;
    }

    //increase count of grids found
    num_of_grids++;

    return NULL;
}

//this method is primarily to check whether a given puzzle is complete
//that is if the puzzle has all numbers present in the puzzle (besides 0) whether they are valid or invalid it would be complete
//ensure completion by using this method. 
bool isPuzzleComplete(int** puzzle, int size) {
    //this is pretty simple, we loop through the puzzle where i is row and j is col and simply check each pos
    for (int i = 1; i <= size; i++) 
    {
        for (int j = 1; j <= size; j++) 
        {
            if (puzzle[i][j] == 0) //if any pos is 0 that means the puzzle is incomplete. return false
            {
                return false;
            }
        }
    }
    //if we reach this point it means that the puzzle is indeed valid so return true; 
    return true;
}


//This is the extra credit method that is used to complete the incomplete puzzles.
//if a puzzle has zeroes this method would go and fill those zeroes such that the puzzle would be complete and valid.
void* fillZeros(void* thread_data) {

    //iniitalize appropriate data members
    parameters* thread = (parameters*) thread_data;
    int row = thread->row;
    int size = thread->length;
    int** puzzle = thread->grid;

    //loop through puzzle
    for (int col = 1; col <= size; col++) 
    {
        //if we find a 0 aka a number that needs to be filled
        if (puzzle[row][col] == 0) 
        {
            //we take the same approach using the found array to find the missing number
            int found_values[size + 1];

            //set values to 0 indicating not found
            for (int k = 0; k <= size; k++) 
            {
                found_values[k] = 0;
            }

            //This part of the code essentially checks row, col, and grid and finds all the numbers that are present in these arrays to help find the missing number

            // Check row and mark the found values in the row
            for (int k = 1; k <= size; k++)
            {
                found_values[puzzle[row][k]] = 1;
            }

            // Check column and mark the found values in the column
            for (int k = 1; k <= size; k++) 
            {
                found_values[puzzle[k][col]] = 1;
            }

            int grid_size = sqrt(size);

            //calculate the starting position of the grid that the current cell belongs to 
            int grid_start_row = ((row - 1) / grid_size) * grid_size + 1;
            int grid_start_col = ((col - 1) / grid_size) * grid_size + 1;

            // Check grid and mark the found values in the grid.
            for (int m = grid_start_row; m < grid_start_row + grid_size; m++) 
            {
                for (int n = grid_start_col; n < grid_start_col + grid_size; n++) 
                {
                    found_values[puzzle[m][n]] = 1;
                }
            }

            // Now that we have marked the found values, the index of the value that is missing is going to be the missing number
            for (int k = 1; k <= size; k++) 
            {
                if (found_values[k] == 0) //if the value is missing, then the index of it would be the missing number
                {
                    puzzle[row][col] = k;
                    break;
                }
            }
        }
    }

    return NULL;
}

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}


// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  //if the puzzle is not complete which means that it has zeroes
  if (!complete) 
  {
        pthread_t threads[sudokuSize]; //create sudokuSize number of threads

        for (int i = 1; i <= sudokuSize; i++)  //loop through and assign appropriate data
        {
            parameters* thread = (parameters*)malloc(sizeof(parameters));
            thread->row = i;
            thread->col = -1;
            thread->grid = grid;
            thread->length = sudokuSize;

            if (pthread_create(&threads[i], NULL, fillZeros, (void*)thread)) //create the threads to call fillZeroes on each row of puzzle
            {
                printf("Failed to create fillZero threads \n");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 1; i <= sudokuSize; i++) //wait for threads to complete
        {
            pthread_join(threads[i], NULL);
        }

        checkPuzzle(sudokuSize, grid, &complete, &valid); //call check puzzle once more to update status of complete and valid once zeroes have been filled.
  }
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
