#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include "utility.h"
#include "StackArray.h"

/// A class to tokenize a given input string.
class Tokenize
{
private:
    int index = 0;
    const String kInput_string;
    int size_of_input = 0;

public:
    Tokenize(const String &input_string)
        : kInput_string(input_string)
    {
        size_of_input = strlen(kInput_string.c_str());
    }

    /**
     * Retrieves the next token from the input string.
     *
     * @return The next token as a String.
     */
    String GetNext();
    ~Tokenize() = default;
};

#endif