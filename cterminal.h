#ifndef CTERMINAL_H
#define CTERMINAL_H

#define MAX_CHARS 80
#define MAX_FIELDS 5

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------
typedef struct _USER_DATA
{
    char buffer[MAX_CHARS + 1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

bool alphaNum (char c, bool check);
bool notAlphaNum (char c);
bool alphaChar(char c);
bool numerChar(char c);
void getsUart0(USER_DATA* data);
bool cmP(char *s1, char *s2);
void parseFields(USER_DATA* data);
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments);
int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber);
char* getFieldString(USER_DATA* data, uint8_t fieldNumber);
uint8_t findInt(USER_DATA* data, uint8_t position);

#endif
