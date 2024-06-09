#include <stdlib.h>
#include <ctype.h>
#include "StackArray.h"
#include "tokenize.h"

/**
 * Check if a char is digit.
 *
 * @param chr Input char
 * @return True if the char is digit
 */
static bool IsDigit(char c)
{
    return (c >= '0' && c <= '9');
}

/**
 * Retrieves the next token from the input string.
 *
 * @return The next token as a String
 */
String Tokenize::GetNext()
{
    if (index == size_of_input)
    {
        return "";
    }

    String buffer{};
    for (index; index < size_of_input; index++)
    {
        if (IsDigit(kInput_string[index]))
        {
            buffer += kInput_string[index];
        }
        else
        {
            if (strlen(buffer.c_str()) == 0)
            {
                buffer += kInput_string[index];
                index++;
                return buffer;
            }

            return buffer;
        }
    }
    return buffer;
}