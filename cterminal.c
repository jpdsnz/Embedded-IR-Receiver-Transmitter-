#include <stdint.h>
#include <stdbool.h>
#include "uart0.h"
#include "cterminal.h"

// Check for alphanumeric char
bool alphaNum (char c, bool check)
{
  if ( (c >= 'a' && c <= 'z') ||  ( c >='0' && c <= '9')  || (c >= 'A' && c <= 'Z') && (!check) )
  {
    return true;
  }

  else
  {
    return false;
  }
}

// Check for non alphanumric char
bool notAlphaNum (char c)
{
  if ( !(c >= 'a' && c <= 'z') || !( c >='0' && c <= '9') || !(c >= 'A' && c <= 'Z')   )
  {
    return true;
  }

  else
  {
    return false;
  }
}

// Check for alpha char
bool alphaChar(char c)
{
  if ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') )
    return true;

  else
  {
    return false;
  }
}

// Check for numeric char
bool numerChar(char c)
{
  if ( c >='0' && c <= '9')
  {
    return true;
  }

  else
  {
    return false;
  }
}

// Compare two char array
bool cmP(char *s1, char *s2)
{
    uint8_t c = 0;
    while (s1[c] != '\0' || s2[c] != '\0')
    {
        if(s1[c]  != s2[c])
            return false;

        c++;
    }
    return true;
}

//j rec info from uart
void getsUart0(USER_DATA* data)
{
    uint8_t count = 0;


    char c;
    while(true)
    {
        c = getcUart0();

        /
        if((c == 8 || c == 127) && count > 0) // if backspace and count > 0 then dec count
        {
            count--;
        }

        else if(c == 13) // add null if c is carriage
        {
            data->buffer[count] = '\0';
            return;
        }

        else if(c >= 32) // if c is space, store char
        {
            data->buffer[count] = c;
            count++;


            if(count == MAX_CHARS)
            {
                data->buffer[count] = '\0';
                return;
            }
        }
    }
}


void parseFields(USER_DATA* data)
{
    bool check = false;
    uint8_t i = 0;
    for(i = 0; (data->buffer[i] != '\0') && (data->fieldCount < MAX_FIELDS); i++)
    {
        // split only alphanum chars
        if( ((data->buffer[i] >= 'a' && data->buffer[i] <= 'z') || (data->buffer[i] >= '0' && data->buffer[i] <= '9') || (data->buffer[i] >= 'A' && data->buffer[i] <= 'Z')) && !check )
        {
            check = true;
            data->fieldPosition[data->fieldCount] = i; // checking again
            if(data->buffer[i] >= '0' && data->buffer[i] <= '9')
                data->fieldType[data->fieldCount++] = 'n'; // = numer char
            else
                data->fieldType[data->fieldCount++] = 'a'; // = alpha char
        }
        // check if not alpha num and write a null if it is
        else if(!(data->buffer[i] >= 'a' && data->buffer[i] <= 'z') && !(data->buffer[i] >= '0' && data->buffer[i] <= '9') && !(data->buffer[i] >= 'A' && data->buffer[i] <= 'Z'))
        {
            check = false;
            // Change delim type to NULL
            data->buffer[i] = '\0';
        }
    }
    return;
}




// If cmd matches 1st field and num of args is greater than or = to min args
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
    if (data->fieldCount >= minArguments)
    {
        uint8_t count = 0;

        // is buffer a command?
        while (data->buffer[count] != '\0' || strCommand[count] != '\0')
        {
            if(data->buffer[count] != strCommand[count])
            {
                return false;
            }
            count++;
        }
        return true;
    }
    else
    {
        return false;
    }
}


int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber) //
{
    if( (fieldNumber < MAX_FIELDS) && (fieldNumber < data->fieldCount) && (data->fieldType[fieldNumber] == 'n') )
        return data->fieldPosition[fieldNumber];
    return 0;
}



// Return pointer to string if field is in range
char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
    if( (fieldNumber < MAX_FIELDS) && (fieldNumber < data->fieldCount) && (data->fieldType[fieldNumber] == 'a') )
        return data->buffer + data->fieldPosition[fieldNumber];
    return 0;
}

// gets integer from pter
uint8_t findInt(USER_DATA* data, uint8_t position)
{
    uint8_t ret = 0;
    while(data->buffer[position] != '\0')
    {
        ret = (ret * 10) + (data->buffer[position] - 48); //Convert to ascii value
        position++;
    }
    return ret;
}
