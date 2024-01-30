#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<fcntl.h>


struct movies;
struct movies *createMovies(char* currline);
struct movies* processFile(char* filename);
void parseMovies(struct movies* list);
void printmessg();
void DoIt(int choice, struct movies* list);

struct movies {
    char* title;
    char* year;
    char* languages;
    char* rate;
    struct movies* next;
};

struct movies *createMovies(char* currline) {
    struct movies* currMovie = malloc(sizeof(struct movies));
    // For use with strtok_r
    char *saveptr;

    // The first token is the title
    char *token = strtok_r(currline, ",", &saveptr);
    currMovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->title, token);

    // The next token is the year
    token = strtok_r(NULL, ", ", &saveptr);
    currMovie->year = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->year, token);

    // The next token is the languages
    token = strtok_r(NULL, ", ", &saveptr);
    currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->languages, token);

    // The last token is the rate
    token = strtok_r(NULL, "\n", &saveptr);
    currMovie->rate = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->rate, token);

    // Set the next node to NULL in the newly created movie entry
    currMovie->next = NULL;

    return currMovie;
}

struct movies* processFile(char* filename) {
    FILE *fptr = fopen(filename, "r");
    
    char *currline = NULL;
    size_t len = 0;
    ssize_t nread;
    char *token;

    struct movies *head = NULL;
    struct movies *tail = NULL;

    getline(&currline, &len, fptr);
    while ((nread = getline(&currline, &len, fptr)) != -1)
    {
        // printf("%s", currline);
        // Get a new movie node corresponding to the current line
        struct movies *newNode = createMovies(currline);
        if (head == NULL)
        {
            // This is the first node in the linked link
            // Set the head and the tail to this node
            head = newNode;
            tail = newNode;
        }
        else
        {
            // This is not the first node.
            // Add this node to the list and advance the tail
            tail->next = newNode;
            tail = newNode;
        }
    }
    free(currline);
    fclose(fptr);
    return head;
}

void parseMovies(struct movies* list) {
    int num = 0;
    while (list != NULL) {
        num++;
        list = list->next;
    }
    printf("Processed file movies_sample_1.csv and parsed data for %d movies\n", num);
}

void printmessg() {
    printf("\n\n1. Show movies released in the specified year\n2. Show highest rated movie for each year\n3. Show the title and year of release of all movies in a specific language\n4. Exit from the program\n\nEnter a choice from 1 to 4: ");
}

void DoIt(int choice, struct movies* list) { 
    // Main funciton here, execute different functions based on passed in argument "choice"
    int year, button = 0;
    if (choice == 1) {
        // first option: store and parse year, and print result
        printf("Enter the year for which you want to see movies: ");
        scanf("%d", &year);
        while (list != NULL) {
            if (atoi(list->year) == year) {
                printf("\n%s",list->title);
                button = 1;
            }
            list = list->next;
        }
        if (button == 0) {
            printf("No data about movies released in the year %d\n", year);
        }
        
    } else if (choice == 2) { 
        // Create two arrays of index 0 - 121, the content is the rate, and title seperatly.
        float arr[122] = {-1};
        char* titleName[122] = {0};

        while (list != NULL) { 
            if (atof(list->rate) >= arr[atoi(list->year) - 1900]) {
                arr[atoi(list->year) - 1900] = atof(list->rate);
                titleName[atoi(list->year) - 1900] = list->title;
            }
            list = list->next;
        }

        for (int i = 0; i < 122; i++) {
            if (arr[i] > 0) {
                printf("%d %.1f %s\n", (1900 + i), arr[i], titleName[i]);
            }
        }
    } else if (choice == 3) {
        
        char movieName[20] = {0}; 
        char* token = NULL;
        int button = 0;
        char* titleArr[122] = {NULL};
        printf("Enter the language for which you want to see movies: ");
        scanf("%s", movieName);

        while (list != NULL) {
            // keep strtoking the languages to see if it matches with movieName that user wants.
            token = strtok(list->languages, "[];");
            while (token != NULL) {
                if (strcmp(token, movieName) == 0) {
                    button = 1;
                    titleArr[atoi(list->year) - 1900] = list->title;
                }
                token = strtok(NULL, "[];");
            }
            list = list->next;
        }

        if (button == 0) {
            // button = 0 if no it never matches.
            printf("No data about movies released in %s", movieName);
            return;
        }
        
        for (int i = 0; i < 122; i++) {
            // print year and title
            if (titleArr[i] != NULL) {
                printf("%d %s\n", (1900+i), titleArr[i]);
            }
        }
        
    }
}

int main(int argc, char *argv[]) {
    int choice;
    if (argc < 2) {
        printf("Provide the name of the file!\n");
    }
    struct movies *list = processFile(argv[1]);
    parseMovies(list);
    printmessg();
    scanf("%d", &choice);
    while (choice < 1 || choice > 4) {
        fprintf(stderr, "Wrong input, please enter again\nEnter a choice from 1 to 4: ");
        scanf("%d", &choice);
    }
    while (choice != 4) {
        DoIt(choice, list);
        printmessg();
        scanf("%d", &choice);
    }
    return 0;
}
