// Below is a C puzzle:
// Write a computer program to run on a Linux computer using the GNU compiler.
// A comma-delimited file has two columns: timeA, timeB. 
// Both columns are times in hh:mm [a|p]m. (hh = 1-12, mm = 1 - 59) or ISO format (yyyy-mm-dd hh24:mm).

// Write a program to read the file:
//       - for each line print the time that is earlier (assume same day for each line, and also same timezone for each time). 
//         The records should be output in the order they are in the file.
//       - after the initial print, print for each row all the times that are later without reading the file again. 
//         The records should be in reverse order of the file order.

// Program should use linked lists.  It should compile and run.

// Include the source code and any instructions necessary to build your program.
// ------------------------------------------------------------------------------------------
// For simplicity the following assumptions are presumed:
//
// 1. As I visualize this file it shall look as follows - the code I wrote shall process such file 
//    (number of spaces between 1st and 2 time is not important for the current algorithm):
//
//  TimeA              TimeB
// ---------------------------------------
// 01:23 am,           17:48 pm
// 02:34 pm,           11:48 am
// 12:56 pm,           11:35 am
// 12:45 pm,           2018-08-02 17:23
// 2018-08-02 07:40,   2018-08-02 13:06
// 2018-08-02 06:12,   12:44 am
//
// 2. The hours and the minutes are always represented by 2 digits, i.e. 1 pm will be 01, and 4 minutes will be as 04.
// 3. Only time has the column ':' character separating 2 digits-hours and 2-digits minutes, without any spaces between 
//    the column and the digits to the left and to the right of it.
// 4. The comma ',' appears only once in a string and follows the 1st time immediately, without any spaces or other 
//    characters in between.
// 5. Every line in the file has 2 valid times
// 6. The input file fith the times is located in the same folder as the executable
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTRLEN 60 // The length of each string shall be less than 40 characters even for the 2 longest formats, but just in case, reserve more space 
#define MAXTIMELEN 9 // The maximum length of time: xx:yy pm

char time_str[9]; // Storage for the longest time format

// A node for a doubly-linked list
struct node
{
    char time_arr[2][MAXTIMELEN]; // Storage for the longest time format: index 0 - earlier time, index 1 - later time
    struct node *prev;
    struct node *next;
};
struct node *head = NULL, *last_node = NULL;

struct node *add_new_node(struct node *last_node, char *str);
void display_earlier(struct node *head);
void display_later(struct node *end_node);

int main ()
{
    FILE *file_ptr;
    char str[MAXSTRLEN];

    file_ptr = fopen("times_file.txt","r");
    if (file_ptr == NULL)
    {
        printf("!!! Error opening file !!!\n");
        return(-1);
    }

    fseek(file_ptr, 0, SEEK_END);
    int length_of_file = ftell(file_ptr);
    if (length_of_file == 0)
    {
        printf("The file with times is empty - nothing to display\n");
        fclose(file_ptr);
        return(0);
    }
    else
    {
        fseek(file_ptr, 0, SEEK_SET); // Reset to the beginning of the file
    }


    while(feof(file_ptr) == 0)
    {   
        if (fgets(str, MAXSTRLEN, file_ptr) != NULL)
        {
            // A line from the file was successfully read.
            last_node = add_new_node(last_node, str);
            if (last_node == NULL)
            {
                fclose(file_ptr); // Something is wrong. The error is printed by the subroutine that determined the error
                return(-1);
            }
        }
    }
    fclose(file_ptr);

    display_earlier(head);
    display_later(last_node);

    return(0);
}


struct node *add_new_node(struct node *last_node, char *str)
{
    char times_str_arr_full[2][MAXTIMELEN] = {"\0", "\0"};
    char *curr_char = str;
    int times[2] = {0}, time_idx = 0, i = 0;
    int str_length = strlen(str);
    struct node *new_node = (struct node*)malloc(sizeof(struct node));

    while ((i < str_length) && (time_idx < 2))
    {
        if (*curr_char == ':')
        {
            char two_dgts[2] = "\0";
            int hours = 0;

            if (((i < (str_length -3)) && (*(curr_char + 3*sizeof(char)) == ','))
                || (i == (str_length - 4)))
            {
                // String continues after the minutes and the 3rd carachter after the ':' is ',' (i.e. this is the 1st time),
                // or string ends right after the minutes (i.e. this is the 2nd time) - this is 24hours format:
                strncpy(times_str_arr_full[time_idx], curr_char - 2*sizeof(char), 5); // Copy all 5 characters for future display
                strncpy(two_dgts, curr_char - 2*sizeof(char), 2 ); // Copy 2 digits of hours into the 2-digits string
                hours = atoi(two_dgts);
                ++curr_char; // advance to the 1st digit after ':'
                strncpy(two_dgts, curr_char, 2 ); // Copy 2 digits of minutes into the 2-digits string
                times[time_idx] = hours*100 + atoi(two_dgts);
                curr_char += 3*sizeof(char); // Advance to the next possible character, skipping single minutes and space or comma
                i += 4;
            }
            else // This is the am/pm format
            {
                // Just as precaution:
                if (i <= (str_length - 6))
                {
                    strncpy(times_str_arr_full[time_idx], curr_char - 2*sizeof(char), 8); // Copy all 8 characters for future display
                    strncpy(two_dgts, curr_char - 2*sizeof(char), 2 ); // Copy 2 digits of hours into the 2-digit string
                    hours = atoi(two_dgts);
                    ++curr_char; // advance to the 1st digit after ':'
                    strncpy(two_dgts, curr_char, 2 ); // Copy 2 digits of minutes into the 2-digits string
                    times[time_idx] = hours*100 + atoi(two_dgts);
                    curr_char += 3*sizeof(char); // advance to the 'p' or a' from "pm" or "am"
                    if ((*curr_char == 'p') && (hours != 12))
                    {
                        times[time_idx] += 1200;
                    }
                    curr_char += 3*sizeof(char); // Advance to the next possible character, skipping 'm' and space or comma
                    i +=7;
                }
                else
                {
                    // Something is wrong: this time is NOT in 24-hour format, but the string ends before am/pm characters
                    printf("ERROR: 12-hour am/pm format is not spelled out in the string!\n");
                    return(NULL);
                }
            }
        time_idx++;

        }
        else // Not a ':' - advance to the next character
        {
            i++;
            curr_char++; 
        } 
    } // End of loop 

    // The string has been traversed, both times are extracted
    if (times[0] > times[1])
    {
        strcpy(new_node->time_arr[0], times_str_arr_full[1]);
        strcpy(new_node->time_arr[1], times_str_arr_full[0]);
    }
    else
    {
        strcpy(new_node->time_arr[0], times_str_arr_full[0]);
        strcpy(new_node->time_arr[1], times_str_arr_full[1]);
    }


    if (last_node == NULL)
    {
        // This is the 1st, head, node: remember it
        new_node->next = NULL;
        new_node->prev = NULL;
        head = new_node;
    }
    else
    {
        last_node->next = new_node;
        new_node->prev = last_node;
        new_node->next = NULL;
    }

    return(new_node);
}

void display_earlier(struct node *head)
{
    printf("display_earlier times in natural order:\n");
    do
    {
        printf(head->time_arr[0]);
        printf("\n");
        head = head->next;
    }
    while (head != NULL);
    printf("\n");
    return;
}

void display_later(struct node *last_node)
{
    printf("display_later times in reverse order:\n");
    do
    {
        printf(last_node->time_arr[1]);
        printf("\n");
        last_node = last_node->prev;
    }
    while (last_node != NULL);
    return;
}
