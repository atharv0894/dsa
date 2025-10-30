#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_STRING_SIZE 256

// --- Data Structures ---
typedef struct Node
{
    char songName[MAX_STRING_SIZE];
    char filePath[MAX_STRING_SIZE];
    struct Node *next;
    struct Node *prev; // Doubly-linked list for next/previous functionality
} Node;

// --- Global Variables ---
Node *playlist = NULL;   // Head of the playlist
Node *nowPlaying = NULL; // Pointer to the current song

// --- Function Prototypes ---
void load_from_csv();
int songExists(Node *head, char *path);
Node *insert(Node *head, char *name, char *path);
void add_to_playlist();
void displayAllSongs(Node *head);
void playMedia(Node *song);
void play_next_song();
void play_prev_song();
void play_from_beginning();
void play_all_songs();
void play_selected_song(); // Added prototype
void free_playlist();
void main_menu();

// --- Main Function ---
int main()
{
    int userChoice = 0;
    system("clear");

    load_from_csv(); // Load existing songs from file on startup

    while (userChoice != -1)
    {
        main_menu();

        char input[10];
        // Use fgets for safer input to prevent buffer overflows
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            // Handle end-of-file or read error
            break;
        }

        if (sscanf(input, "%d", &userChoice) != 1)
        {
            system("clear");
            printf("---- INVALID INPUT ----\n");
            userChoice = 0; // Reset to show menu again
            continue;
        }

        switch (userChoice)
        {
        case 1:
            system("clear");
            displayAllSongs(playlist);
            break;
        case 2:
            system("clear");
            add_to_playlist();
            break;
        case 3:
            system("clear");
            play_from_beginning();
            break;
        case 4:
            system("clear");
            play_prev_song();
            break;
        case 5:
            system("clear");
            play_next_song();
            break;
        case 6:
            system("clear");
            printf("Replaying track.\n");
            playMedia(nowPlaying);
            break;
        case 7:
            system("clear");
            play_all_songs();
            break;
        case 8: // Added case for new function
            system("clear");
            play_selected_song();
            break;
        case -1:
            printf("Exiting player.!\n");
            system("osascript -e 'tell application \"QuickTime Player\" to quit'");
            break;
        default:
            system("clear");
            printf("Please enter a valid option from the menu.\n");
            break;
        }
    }

    free_playlist(); // Free all allocated memory before exiting
    return 0;
}

// --- Function Definitions ---

void main_menu()
{
    printf("\n----------------------------------------------------------------------\n");
    printf("                              MAIN MENU\n");
    printf("----------------------------------------------------------------------\n");
    if (nowPlaying != NULL)
    {
        printf("  NOW PLAYING: %s\n", nowPlaying->songName);
        printf("----------------------------------------------------------------------\n");
    }
    printf("   #   |  Action   \n");
    printf("----------------------------------------------------------------------\n");
    printf("   1   |  Display all available songs\n");
    printf("   2   |  Add a new song to the playlist\n");
    printf("   3   |  Play from the beginning\n");
    printf("   4   |  Play previous track\n");
    printf("   5   |  Play next track\n");
    printf("   6   |  Replay current track\n");
    printf("   7   |  Play all songs (continuous play)\n");
    printf("   8   |  Play a specific song\n"); // Added new option
    printf("  -1   |  Exit music player\n");
    printf("----------------------------------------------------------------------\n");
    printf("                        Enter your choice: ");
}

void load_from_csv()
{
    FILE *fp = fopen("songs.csv", "r");
    if (fp == NULL)
    {
        return; // File doesn't exist yet, which is fine.
    }
    char line[MAX_STRING_SIZE * 2];
    while (fgets(line, sizeof(line), fp))
    {
        char name[MAX_STRING_SIZE], path[MAX_STRING_SIZE];
        if (sscanf(line, "%[^,],%[^\n]", name, path) == 2)
        {
            playlist = insert(playlist, name, path);
        }
    }
    fclose(fp);
    if (playlist != NULL)
    {
        nowPlaying = playlist;
        printf("Playlist loaded from songs.csv!\n");
    }
}

int songExists(Node *head, char *path)
{
    Node *temp = head;
    while (temp != NULL)
    {
        if (strcmp(temp->filePath, path) == 0)
            return 1;
        temp = temp->next;
    }
    return 0;
}

Node *insert(Node *head, char *name, char *path)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL)
    {
        perror("Failed to allocate memory for new song");
        exit(EXIT_FAILURE);
    }
    strcpy(newNode->songName, name);
    strcpy(newNode->filePath, path);
    newNode->next = NULL;
    newNode->prev = NULL;

    if (head == NULL)
    {
        return newNode;
    }

    Node *temp = head;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = newNode;
    newNode->prev = temp;
    return head;
}

void add_to_playlist()
{
    char name[MAX_STRING_SIZE], path[MAX_STRING_SIZE];

    printf("Enter song name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    printf("Enter file path: ");
    fgets(path, sizeof(path), stdin);
    path[strcspn(path, "\n")] = 0;

    if (songExists(playlist, path))
    {
        printf("\nError: This song is already in the playlist.\n");
        return;
    }

    FILE *fp = fopen("songs.csv", "a");
    if (fp == NULL)
    {
        perror("Error opening CSV file");
        return;
    }
    fprintf(fp, "%s,%s\n", name, path);
    fclose(fp);

    playlist = insert(playlist, name, path);
    if (nowPlaying == NULL)
    {
        nowPlaying = playlist;
    }
    printf("\n'%s' has been added to the playlist.\n", name);
}

void displayAllSongs(Node *head)
{
    if (head == NULL)
    {
        printf("🎵 No songs in the playlist!\n");
        return;
    }
    printf("\n🎶 Your Playlist:\n");
    int count = 1;
    Node *temp = head;
    while (temp != NULL)
    {
        printf("%d. %s\n", count++, temp->songName);
        temp = temp->next;
    }
}

// Plays the given song using a system command
void playMedia(Node *song)
{
    if (song == NULL)
    {
        printf("No song selected to play.\n");
        return;
    }

    char command[1024];
    printf("🎵 Now Playing: %s\n", song->songName);

    // This command is for macOS. For Linux use "xdg-open", for Windows use "start"
    sprintf(command,
            "osascript "
            // Closes all currently open songs to prevent simultaneous playback.
            "-e 'tell application \"QuickTime Player\" to close every document without saving' "
            "-e 'tell application \"QuickTime Player\" to activate' "
            "-e 'tell application \"QuickTime Player\" to open POSIX file \"%s\"' "
            "-e 'tell application \"QuickTime Player\" to play the front document'",
            song->filePath);

    system(command);
}

// Plays the next song in the list, looping to the start if at the end.
void play_next_song()
{
    if (playlist == NULL)
    {
        printf("The playlist is empty.\n");
        return;
    }

    if (nowPlaying == NULL)
    {
        // If nothing is playing, "next" will start from the beginning
        nowPlaying = playlist;
    }
    else if (nowPlaying->next == NULL) // We are at the last song
    {
        printf("Reached end of playlist, looping to beginning...\n");
        nowPlaying = playlist; // Go back to the head (first song)
    }
    else // We are in the middle of the list
    {
        nowPlaying = nowPlaying->next; // Just go to the next one
    }

    playMedia(nowPlaying);
}

// Plays the previous song in the list, looping to the end if at the start.
void play_prev_song()
{
    if (playlist == NULL)
    {
        printf("The playlist is empty.\n");
        return;
    }

    Node *last = playlist; // Start at the head to find the last node

    if (nowPlaying == NULL)
    {
        // If nothing is playing, "previous" will start from the end
        printf("Looping to end of playlist...\n");
        while (last->next != NULL) // Traverse to the end
        {
            last = last->next;
        }
        nowPlaying = last;
    }
    else if (nowPlaying->prev == NULL) // We are at the first song
    {
        printf("At first song, looping to end of playlist...\n");
        while (last->next != NULL) // Traverse to the end
        {
            last = last->next;
        }
        nowPlaying = last; // Set 'nowPlaying' to the last node
    }
    else // We are in the middle of the list
    {
        nowPlaying = nowPlaying->prev; // Just go to the previous one
    }

    playMedia(nowPlaying);
}

// Sets the current song to the first song in the playlist and plays it
void play_from_beginning()
{
    if (playlist == NULL)
    {
        printf("The playlist is empty. Add some songs first.\n");
        return;
    }
    nowPlaying = playlist;
    playMedia(nowPlaying);
}

// Iterates through the playlist and plays all songs sequentially
void play_all_songs()
{
    if (playlist == NULL)
    {
        printf("The playlist is empty.\n");
        return;
    }

    printf("\n--- Playing all songs ---\n");
    Node *temp = playlist; // Start from the beginning

    while (temp != NULL)
    {
        nowPlaying = temp;
        playMedia(nowPlaying); // Play the current song

        if (temp->next != NULL) // Check if there's a next song
        {
            printf("Press ENTER to play next song ('%s') or type 'stop' to return to menu: ", temp->next->songName);
            char choice[10];
            fgets(choice, sizeof(choice), stdin);

            if (strncmp(choice, "stop", 4) == 0)
            {
                break; // Exit the loop if user types 'stop'
            }
        }
        temp = temp->next; // Move to the next song
    }
    printf("\n--- Finished playing all songs ---\n");
}

// *** NEW FUNCTION ***
// Plays a specific song selected by the user
void play_selected_song()
{
    if (playlist == NULL)
    {
        printf("The playlist is empty. Add some songs first.\n");
        return;
    }

    // 1. Display all songs so the user can choose
    displayAllSongs(playlist);
    printf("----------------------------------------------------------------------\n");
    printf("Enter the number of the song you want to play (or 0 to cancel): ");

    // 2. Get user input
    char input[10];
    int songChoice = 0;
    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        return; // Handle read error
    }

    if (sscanf(input, "%d", &songChoice) != 1)
    {
        printf("Invalid input. Please enter a number.\n");
        return;
    }

    if (songChoice == 0)
    {
        printf("Selection cancelled.\n");
        return;
    }

    // 3. Find the song in the list
    Node *temp = playlist;
    int count = 1;
    // Traverse the list until we find the matching number or reach the end
    while (temp != NULL && count < songChoice)
    {
        temp = temp->next;
        count++;
    }

    // 4. Play the song or show an error
    if (temp != NULL && count == songChoice)
    {
        // Found it!
        nowPlaying = temp;
        playMedia(nowPlaying);
    }
    else
    {
        // User entered a number that is too high or invalid
        printf("Invalid song number. No song found at position %d.\n", songChoice);
    }
}

// Frees all nodes in the linked list to prevent memory leaks
void free_playlist()
{
    Node *current = playlist;
    Node *next_node;
    while (current != NULL)
    {
        next_node = current->next;
        free(current);
        current = next_node;
    }
    playlist = NULL;
}