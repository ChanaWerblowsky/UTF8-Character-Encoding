#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


// utf8 encoding/decoding and related functions
int my_utf8_encode(unsigned char *input, unsigned char *output);
int my_utf8_decode(unsigned char *input, unsigned char *output);
int my_utf8_strlen(unsigned char *string);
int my_utf8_check(unsigned char *string);
unsigned char *my_utf8_charat(unsigned char *string, int index);
int my_utf8_strcmp(unsigned char *string1, unsigned char *string2);
int my_utf8_strcat(unsigned char *dest, unsigned char *src);
int my_utf8_strreverse(unsigned char *input, unsigned char *output);

// helper functions
unsigned char hexdigit_to_ascii(unsigned int h);
unsigned char *hex_to_ascii(unsigned int h);
int asciichar_to_hex(unsigned char c);
unsigned int ascii_to_hex(unsigned char *c);
int my_utf8_charsize(unsigned char c);
int my_utf8_check_codepoint(unsigned int codepoint);

// test functions
void test_encode(unsigned char *input, unsigned char *output, unsigned char *expected, char *test_name);
void testall_encode(void);
void test_decode(unsigned char *input, unsigned char *output, unsigned char *expected, char *test_name);
void testall_decode(void);
void test_strlen(unsigned char *string, int expected);
void testall_strlen(void);
void test_check(unsigned char *string, int expected);
void testall_check(void);
void test_charat(unsigned char *string, int index, unsigned char *expected);
void testall_charat(void);
void test_strcmp(unsigned char *string1, unsigned char *string2, bool expected);
void testall_strcmp(void);
void test_strcat(unsigned char *dest, unsigned char *src, unsigned char *expected);
void testall_strcat(void);
void test_strreverse(unsigned char *input, unsigned char *output, unsigned char *expected);
void testall_strreverse(void);

int main() {

    testall_encode();
    testall_decode();
    testall_strlen();
    testall_check();
    testall_charat();
    testall_strcmp();
    testall_strcat();
    testall_strreverse();

    return 0;
}


// Encodes the given unicode codepoint as a UTF8 character
// if an invalid character or codepoint is encountered, the output string is terminated there
int my_utf8_encode(unsigned char *input, unsigned char *output)
{
    bool success = true;
    int j = 0;    // index in output string
    while (*input != '\0'){

        if (input[0] == '\\') {

            int num_digits;
            if (input[1] == 'u')
                num_digits = 4;

            else if (input[1] == 'U')
                num_digits = 8;

            else{
                output[j++] = input[0];
                input++;
                continue;
            }

            // If we're at a \u or \U:
            // make sure there are at least 4 or at least 8 characters following
            bool enough_chars = true;
            for (int i = 2; i < num_digits+2; i++)
                if (input[i] == '\0')
                    enough_chars = false;
            if (!enough_chars){
                success = false;
                printf("Fewer than %d characters following escape sequence '\\%c'!\n", num_digits, input[1]);
                break;
            }

            // if there are enough characters following:
            // create a string consisting of the following 4 or 8 characters
            unsigned char codepoint_str[num_digits+1];
            int i;
            for (i = 0; i < num_digits; i++){
                codepoint_str[i] = input[i + 2];
            }
            codepoint_str[i] = '\0';

            // convert the string into a 'matching' int
            unsigned int codepoint = ascii_to_hex(codepoint_str);

            // if invalid character following the \u, terminate the output string
            if (codepoint == -1){
                success = false;
                printf("Invalid character following \\%c!\n", input[1]);
                break;
            }

            // check that the codepoint is within a valid range
            bool invalid_codepoint = my_utf8_check_codepoint(codepoint);
            if (invalid_codepoint){
                success = false;
                break;
            }

            if (num_digits == 4){
                // determine in which range this codepoint lies; apply correct encoding algorithm:
                unsigned int left_byte = codepoint >> 8;     // isolate the two hex bytes
                unsigned int right_byte = codepoint & 0x00ff;

                // if less than U+00a0, only U+0024, U+0040, and U+0060 are valid
                if (codepoint <= 0x007f) {
                    output[j++] = right_byte;
                }
                else if (codepoint <= 0x07ff) {
                    output[j++] = 0xc0 + ((left_byte << 2) + (right_byte >> 6));
                    output[j++] = 0x80 + (right_byte & 0x3f);
                }
                else if (codepoint <= 0xffff) {  // if within invalid range
                    output[j++] = 0xe0 + (left_byte >> 4);
                    output[j++] = 0x80 + ((left_byte & 0x0f) << 2) + (right_byte >> 6);
                    output[j++] = 0x80 + (right_byte & 0x3f);
                }
            }

            else {  // num_digits == 8
                unsigned int left_byte = codepoint >> 16;
                unsigned int mid_byte = (codepoint >> 8) & 0x0000ff;
                unsigned int right_byte = codepoint & 0x0000ff;

                if (codepoint >= 0x10000 && codepoint <= 0x10ffff) {
                    output[j++] = 0xf0 + (left_byte >> 2);
                    output[j++] = 0x80 + ((left_byte << 4) + (mid_byte >> 4));
                    output[j++] = 0x80 + ((mid_byte << 2) & 0x3f) + (right_byte >> 6);
                    output[j++] = 0x80 + (right_byte & 0x3f);
                }
            }

            // move to next character in input string
            input += num_digits+2;
        }

        // if here we're not at a '\', '\u' or '\U'
        // if the current char is an ascii character:
        else if (input[0] >= 0 && input[0] <= 127){
            output[j++] = input[0];
            input++;
        }

        // if not, missing preceding \u --> terminate the output string
        else{
            success = false;
            printf("Missing '\\u' before non-ascii character '%c'!\n", input[0]);
            break;
        }
    }
    output[j++] = '\0';

    if (success)
        return 0;
    return 1;
}

// Decodes a properly encoded utf8 string into a hexadecimal universal character
int my_utf8_decode(unsigned char *input, unsigned char *output){

    // first, check that the input string is a valid UTF8 encoded string
    bool invalid = my_utf8_check(input);
    if (invalid){
        *output = '\0';
        return 1;
    }

    int cur_char_len = 0;   // number of bytes that a given character takes up
    int j = 0;              // output string index counter

    while (*input != '\0'){

        // if it's an ascii char, simply copy it into the output array
        if (input[0] >= 0x00 && input[0] < 0x80) {
            cur_char_len = 1;
            output[j++] = input[0];
        }
        else {
            // put in \ (to be followed by 'u' or 'U') to signify that this is a universal character
            output[j++] = '\\';

            if (input[0] >= 0xc0 && input[0] < 0xe0) {
                cur_char_len = 2;   // this char is encoded in two bytes

                // here's where the utf8 decoding happens
                unsigned char a = input[0] - 0xc0;
                unsigned char b = input[1] - 0x80;

                int hex_byte1 = a >> 2;
                int hex_byte2 = ((a << 6) + b) & 0x00ff;

                // both the bytes as one int:
                int hex_codepoint = (hex_byte1 << 8) + hex_byte2;

                // convert the 2 hex bytes into 4 ascii characters
                unsigned char *ascii_codepoint = hex_to_ascii(hex_codepoint);

                // and copy them into the output string
                output[j++] = 'u';
                for (int i = 0; i < 4; i++){
                    output[j++] = ascii_codepoint[i];
                }
                free(ascii_codepoint);

            } else if (input[0] >= 0xe0 && input[0] < 0xf0) {
                cur_char_len = 3;

                // here's where the utf8 decoding happens
                // first byte of decoded string consists of last four bits in first byte of encoded string concatenated with its middle 4 bits
                int a = (input[0] - 0xe0) << 4;
                int b = (input[1] - 0x80);
                int c = (input[2] - 0x80);

                int hex_byte1 = a + (b >> 2);
                int hex_byte2 = ((b << 6) + c) & 0x00ff;

                // both the bytes as one int:
                int hex_codepoint = (hex_byte1 << 8) + hex_byte2;

                // convert the 2 hex bytes into 4 ascii characters
                unsigned char *ascii_codepoint = hex_to_ascii(hex_codepoint);

                // and copy them into the output string
                output[j++] = 'u';
                for (int i = 0; i < 4; i++){
                    output[j++] = ascii_codepoint[i];
                }
                free(ascii_codepoint);

            } else if (input[0] >= 0xf0 && input[0] < 0xf8){
                cur_char_len = 4;

                // here's where the utf8 decoding happens
                int a = (input[0] - 0xf0) << 2;
                int b = (input[1] - 0x80);
                int c = (input[2] - 0x80);
                int d = (input[3] - 0x80);

                int hex_byte1 = a + (b >> 4);
                int hex_byte2 = ((b << 4) + (c >> 2)) & 0x00ff;
                int hex_byte3 = ((c << 6) + d) & 0x00ff;

                // all the bytes as one int:
                int hex_codepoint = (hex_byte1 << 16) + (hex_byte2 << 8) + hex_byte3;

                // convert the 3 hex bytes into 6 ascii characters
                unsigned char *ascii_codepoint = hex_to_ascii(hex_codepoint);

                output[j++] = 'U';
                output[j++] = '0';
                output[j++] = '0';
                for (int i = 0; i < 6; i++){
                    output[j++] = ascii_codepoint[i];
                }
                free(ascii_codepoint);
            }
        }
        input += cur_char_len;
    }
    output[j] = '\0';   // end-of-string flag
    return 0;
}

// Returns the number of utf8 encoded characters (including ascii characters) in the string
// returns 0 if string is empty or invalid utf8
int my_utf8_strlen(unsigned char *string)
{
    int invalid = my_utf8_check(string);
    if (invalid)
        return 0;

    int length = 0;
    int cur_char_len;

    // loop through the input string, step-size determined by the size of the character
    while (*string != '\0'){
        length++;

        // determine how many bytes this character takes up
        cur_char_len = my_utf8_charsize(*string);
        string += cur_char_len;     // and move to the next character
    }
    return length;
}

// Returns 0 if the input string is a valid utf8-encoded string, and 1 otherwise
int my_utf8_check(unsigned char *string) {

    // first determine the number of bytes in the input string
    int length = 0;
    unsigned char *ptr = string; // new pointer so that we don't lose the original
    while (*ptr != '\0'){
        length++;
        ptr++;
    }

    // then loop through string again, checking whether each character is valid
    int cur_char_len;
    for(int i = 0; i < length; i+= cur_char_len){
        unsigned int bytes_rem = length - i - 1;   // number of bytes that remain in the string after this one

        // Range #1 (less than 0x08 --> ascii character)
        if (string[i] < 0x80) {
            cur_char_len = 1;
        }

        // Range #2
        else if (string[i] >= 0xc0 && string[i] < 0xe0) {

            // if the following byte is not a valid continuation byte
            if (!(string[i+1] >= 0x80 && string[i+1] < 0xc0))
                return 1;

            cur_char_len = 2;
        }

        // Range #3
        else if (string[i] >= 0xe0 && string[i] < 0xf0) {
            if (bytes_rem < 2)
                return 1;

            // if the following 2 bytes are not valid continuation bytes
            if (!(string[i+1] >= 0x80 && string[i+1] < 0xc0 && string[i+2] >= 0x80 && string[i+2] < 0xc0))
                return 1;

            cur_char_len = 3;
        }

        // Range #4
        else if (string[i] >= 0xf0 && string[i] < 0xf8) {
            if (bytes_rem < 3)
                return 1;

            // if the following 3 bytes are not valid continuation bytes
            if (!(string[i+1] >= 0x80 && string[i+1] < 0xc0 && string[i+2] >= 0x80 && string[i+2] < 0xc0 &&
                    string[i+3] >= 0x80 && string[i+3] < 0xc0))
                return 1;

            cur_char_len = 4;
        }
        else
            return 1;
    }
    return 0;
}

// Returns the utf-encoded character at the given index, or NULL if invalid utf8 input string or invalid index
unsigned char *my_utf8_charat(unsigned char *string, int index){

    // if the input string is not a valid utf8 string or the desired index is too large, return NULL
    bool invalid = my_utf8_check(string);
    if (invalid)
        return NULL;

    int length = my_utf8_strlen(string);
    if (index >= length)
        return NULL;

    int cur_pos = 0;        // current position in the input string
    int cur_char_len = 0;   // number of bytes that the current character takes up

    // loop through string until reach the desired index
    while (cur_pos <= index){

        // determine how many bytes this character takes up
        cur_char_len = my_utf8_charsize(*string);

        // move to next character only if we're not already at the desired character
        if (cur_pos < index)
            string += cur_char_len;
        cur_pos++;
    }

    // now, the first element in the string is the desired character (but it might be spread over multiple memory locations)
    unsigned char *ans = (unsigned char*) malloc(cur_char_len);     // build up answer string
    int i;
    for (i = 0; i < cur_char_len; i++)
        ans[i] = string[i];
    ans[i] = '\0';  // end-of-string flag

    return ans;
}

// Returns 0 if the utf8-encoded string1 and string2 are identical, and 1 otherwise (or if invalid input)
int my_utf8_strcmp(unsigned char *string1, unsigned char *string2) {

    // check that both input strings are valid utf8-encoded strings
    bool invalid1 = my_utf8_check(string1);
    bool invalid2 = my_utf8_check(string2);
    if (invalid1 || invalid2){
        printf("Invalid utf8 string passed to strcmp()!\n");
        return 1;
    }

    // check whether string1 == string2
    while (*string1 != '\0' && *string2 != '\0' && *string1 == *string2) {
        string1++;
        string2++;
    }

    // if here, either we've found a non-matching character or we've reached the end of string1 or string2
    if (*string1 == *string2)   // if we've reached the end of both strings, string1 == string2
        return 0;

    return 1;               // otherwise, string1 != string2
}

// Concatenates source string to the end of the destination string if both are valid utf8 strings
// upon encountering invalid input string, leaves the input strings as is and returns 1
int my_utf8_strcat(unsigned char *dest, unsigned char *src){

    int invalid_src = my_utf8_check(src);
    int invalid_dest = my_utf8_check(dest);

    if (invalid_src || invalid_dest)
        return 1;

    // move to the end of the destination string
    while(*dest != '\0'){
        dest++;
    }

    // and copy the source string here
    int i = 0;
    while (src[i] != '\0'){
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0'; // end-of-string flag
    return 0;       // result is in dest
}

// Reverses a utf8 encoded string
int my_utf8_strreverse(unsigned char *input, unsigned char *output){

    int invalid = my_utf8_check(input);
    if (invalid){
        *output = '\0';
        return 1;
    }

    // determine length of input (total number of bytes)
    int length = 0;
    unsigned char *input_ptr = input;
    while (*input_ptr != '\0'){
        length++;
        input_ptr++;
    }

    // for each char, determine many bytes it takes up and put it at the rightmost available position in output
    int cur_char_len;
    int i;
    for (i = 0; i < length; i += cur_char_len){

        // determine how many bytes this character takes up
        cur_char_len = my_utf8_charsize(input[i]);

        int output_index = length - i - cur_char_len;   // starting index in output string

        // copy each byte belonging to this char into the correct position in the output string
        for (int j = 0; j < cur_char_len; j++){
            output[output_index++] = input[i + j];
        }
    }
    output[length] = '\0';  // end-of-string flag
    return 0;
}


/// Helper functions
// converts 1 hex digit to its ascii representation
unsigned char hexdigit_to_ascii(unsigned int h){

    unsigned char ans = '\0';

    // if h is between 0x00 and 0x09, add 0x30 to get its ascii representation
    if (h <= 0x09)
        ans = (unsigned char)(h + 0x30);

        // if h is between 0x0a and 0x0f, add 0x57 to get its (lowercase) ascii representation
    else if (h >= 0x0a && h <= 0x0f)
        ans = (unsigned char)(h + 0x57);

    return ans;
}

// given a 1-4 byte hex value, convert it to a matching ascii string
unsigned char *hex_to_ascii(unsigned int h){

    // first determine how many bytes the hex values contains (assuming that no more than 4 bytes)
    int num_bytes = 0;
    if (h <= 0xFF)
        num_bytes = 1;
    else if (h <= 0xFFFF)
        num_bytes = 2;
    else if (h <= 0xFFFFFF)
        num_bytes = 3;
    else if (h <= 0xFFFFFFFF)
        num_bytes = 4;

    // array to save result
    unsigned char *output = (unsigned char*) malloc(num_bytes * 2);

    unsigned int cur_digit;
    int num_shifts;
    unsigned char cur_digit_as_char;
    int i;
    for(i = 0; i < num_bytes*2; i++){

        // isolate the leftmost hex digit and convert it to ascii
        num_shifts = (num_bytes*2 - i - 1) * 4;
        cur_digit = h >> num_shifts;
        cur_digit_as_char = hexdigit_to_ascii(cur_digit);

        // make sure it was converted properly
        if (cur_digit_as_char == '\0')
            return NULL;

        // if so, save it in output array
        output[i] = cur_digit_as_char;

        // then subtract it from the input hex value and repeat
        h = (h - (cur_digit << num_shifts));
    }
    return output;
}

// returns the hex value of the given ascii character
int asciichar_to_hex(unsigned char c){

    int ans = -1;   // error code (to be reset if no error)
    if (c >= '0' && c <= '9')
        ans = c - 0x30;

    else if (c >= 'a' && c <= 'f')
        ans = c - 0x57;

    else if (c >= 'A' && c <= 'F')
        ans = c - 0x37;

    else
        printf("Cannot convert character '%c' to hex. ", c);

    return ans;
}

// given a series (an even number) of ascii characters, converts them to 'matching' hex int
unsigned int ascii_to_hex(unsigned char *c){

    // length of input string:
    int length = 0;
    while (c[length] != '\0'){
        length++;
    }

    // convert each char to hex, put value in array
    int hex_vals[length];
    int i;
    for (i = 0; i < length; i++){
        int h = asciichar_to_hex(c[i]); // convert this char to hex
        if (h == -1)          // if it can't be converted to hex, return -1
            return -1;

        hex_vals[i] = h;      // otherwise, save the hex val in an array
    }

    // for each pair of hex digits, move the first hex digit 4 bits to the left and add it to the second one
    int num_bytes = length/2;
    int hex_bytes[num_bytes];
    for (int j = 0; j < length; j+=2){   // we're expecting an even number of characters (either 4 or 8)
        hex_bytes[j/2] = (hex_vals[j] << 4) + hex_vals[j+1];
    }

    // finally, combine each of the bytes into 1 hex int
    unsigned int ans = 0;
    int num_shifts;
    for(int k = 0; k < num_bytes; k++){
        num_shifts = (num_bytes - k - 1) * 8;
        ans += hex_bytes[k] << num_shifts;
    }
    return ans;
}


// given the first byte of a utf8-encoded character, returns the number of bytes that it takes up, or 0 if invalid leading byte
// assumes that the continuation bytes have been checked for validity
int my_utf8_charsize(unsigned char c){

    if (c >= 0x00 && c < 0x80)
        return 1;
    if (c >= 0xc0 && c < 0xe0)
        return 2;
    if (c >= 0xe0 && c < 0xf0)
        return 3;
    if (c >= 0xf0 && c < 0xf8)
        return 4;
    return 0;
}

// returns 0 if the given codepoint is a valid universal character and 1 otherwise
int my_utf8_check_codepoint(unsigned int codepoint){

    if (codepoint < 0xa0) {
        if (codepoint != 0x0024 && codepoint != 0x0040 && codepoint != 0x0060) {
            printf("Invalid universal character \\u%x - less than 0xa0\n", codepoint);
            return 1;
        }
        else
            return 0;
    }
    if (codepoint >= 0xd800 && codepoint <= 0xdfff) {
        printf("Invalid universal character \\u%x - between U+d800 and U+dfff\n", codepoint);
        return 1;
    }
    if (codepoint > 0x10ffff){
        printf("Invalid universal character \\u%x - greater than U+10ffff\n", codepoint);
        return 1;
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// TESTING ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void test_encode(unsigned char *input, unsigned char *output, unsigned char *expected, char *test_name){
    bool encode_successful = my_utf8_encode(input, output);  // call the encode() function

    // compare output against correct answer
    int i = 0;
    bool passed = true;
    while(output[i] != '\0' && expected[i] != '\0'){
        if (output[i] != expected[i]){
            passed = false;
            break;
        }
        i++;
    }
    if (expected[i] != output[i])
        passed = false;

    if (passed){
        printf("***TEST PASSED: %s. ", test_name);
        if (encode_successful == 1)
            printf("Invalid input %s dealt with correctly.\n", input);
        else
            printf("Successfully encoded the string %s as %s.\n", input, output);
    }
    else{   // if test failed
        printf("***TEST FAILED: %s. ", test_name);
        if (encode_successful == 1)
            printf("Invalid input %s dealt with incorrectly.\n", input);
        else
            printf("Failed to properly encode the string %s! Actual: %s, Expected: %s\n", input, output, expected);

    }
}

// tests encode() with various test cases
void testall_encode(){

    printf("##############################################################################\n");
    printf("######################### Tests for my_utf8_encode() #########################\n");

    // 1. simple input
    unsigned char input1[] = "\\u05d0\\u05e8\\u05d9\\u05d4";
    unsigned char output1[10];
    unsigned char expected1[] = "\u05d0\u05e8\u05d9\u05d4";
    test_encode(input1, output1, expected1, "Encode - Simple");

    // 2. Range 1 codepoint
    unsigned char input2[] = "\\u0024";
    unsigned char output2[10];
    unsigned char expected2[] = "\u0024";
    test_encode(input2, output2, expected2, "Encode - Range 1 Codepoint");

    // 3. Range 2 codepoint
    unsigned char input3[] = "\\u00A3";
    unsigned char output3[10];
    unsigned char expected3[] = "\u00A3";
    test_encode(input3, output3, expected3, "Encode - Range 2 Codepoint");

    // 4. Range 3 codepoint
    unsigned char input4[] = "\\ud55c";
    unsigned char output4[10];
    unsigned char expected4[] = "\ud55c";
    test_encode(input4, output4, expected4, "Encode - Range 3 Codepoint");

    // 5. Range 4 codepoint (8 digits)
    unsigned char input5[] = "\\U00010348";
    unsigned char output5[10];
    unsigned char expected5[] = "\xf0\x90\x8d\x88";
    test_encode(input5, output5, expected5, "Encode - Range 4 Codepoint");

    // 6. invalid hex digit in codepoint
    unsigned char input6[] = "\\ug0187";
    unsigned char output6[10];
    unsigned char expected6[] = "\0";
    test_encode(input6, output6, expected6, "Encode - Non-hex in Codepoint");

    // 7. incomplete codepoint at the start of the input string
    unsigned char input7[] = "\\u018";
    unsigned char output7[10];
    unsigned char expected7[] = "\0";
    test_encode(input7, output7, expected7, "Encode - Incomplete Codepoint at Beginning");

    // 8. incomplete codepoint at the end of the input string
    unsigned char input8[] = "\\uabcd\\u18";
    unsigned char output8[10];
    unsigned char expected8[] = "\uabcd";
    test_encode(input8, output8, expected8, "Encode - Incomplete Codepoint at End");

    // 9. missing '\u' before non-ascii character
    unsigned char input9[] = "€018";
    unsigned char output9[10];
    unsigned char expected9[] = "\0";
    test_encode(input9, output9, expected9, "Encode - Missing '\\u'");

    // 10. codepoint in protected range (less than 0xa0 and not one of the exceptions)
    unsigned char input10[] = "\\u0035";
    unsigned char output10[10];
    unsigned char expected10[] = "\0";
    test_encode(input10, output10, expected10, "Encode - Codepoint Too Low");

    // 11. codepoint in protected range (between 0xd800 and 0xdfff)
    unsigned char input11[] = "\\ud955";
    unsigned char output11[10];
    unsigned char expected11[] = "\0";
    test_encode(input11, output11, expected11, "Encode - Codepoint in Protected Range");

    // 12. codepoint too large
    unsigned char input12[] = "\\Uffd55955";
    unsigned char output12[10];
    unsigned char expected12[] = "\0";
    test_encode(input12, output12, expected12, "Encode - Codepoint Too Large");

    // 13. backslash without 'u' or 'U'
    unsigned char input13[] = "\\abcdefg\\";
    unsigned char output13[10];
    unsigned char expected13[] = "\\abcdefg\\";
    test_encode(input13, output13, expected13, "Encode - Literal Backslash");
    printf("\n");

    printf("##############################################################################\n");
}

void test_decode(unsigned char *input, unsigned char *output, unsigned char *expected, char *test_name){

    bool decode_successful = my_utf8_decode(input, output);
    bool passed = true;

    int i = 0;
    while(output[i] != '\0' && expected[i] != '\0'){

        if (output[i] != expected[i]){  // if we've found an incorrect character in the output string:
            passed = false;
            break;
            }
        i++;
    }
    if (expected[i] != output[i])
        passed = false;

    // display message
    if (passed){
        printf("***TEST PASSED: %s. ", test_name);
        if (decode_successful == 1)
            printf("Invalid utf8-encoded string %s dealt with correctly.\n", input);
        else
            printf("Successfully decoded the string %s as %s.\n", input, output);
    }
    else{   // if test failed
        printf("***TEST FAILED: %s. ", test_name);
        if (decode_successful == 1)
            printf("Invalid utf8-encoded string %s dealt with incorrectly.\n", input);
        else
            printf("Failed to properly decode the string %s! Actual: %s, Expected: %s\n", input, output, expected);
    }
}

void testall_decode(void){
    printf("######################### Tests for my_utf8_decode() #########################\n");

    // Hebrew Arieh
    unsigned char input1[] = "\u05d0\u05e8\u05d9\u05d4";
    unsigned char output1[30];
    unsigned char expected1[] = "\\u05d0\\u05e8\\u05d9\\u05d4";
    test_decode(input1, output1, expected1, "Decode - Simple");

    // long input string
    unsigned char input2[] = "abcdefg\xd7\x90" "12345678" "\xd7\xa8" "\xf0\x90\x8d\x88" "\xd7\x99\xd7\x94";
    unsigned char output2[60];
    unsigned char expected2[] = "abcdefg\\u05d012345678\\u05e8\\U00010348\\u05d9\\u05d4";
    test_decode(input2, output2, expected2, "Decode - Long Input");

    // empty input string
    unsigned char input3[] = "";
    unsigned char output3[5];
    unsigned char expected3[] = "\0";
    test_decode(input3, output3, expected3, "Decode - Empty Input");

    // invalid utf8 input string
    unsigned char input4[] = "\x01\xD0\x02";  // INVALID utf8 string
    unsigned char output4[10];
    unsigned char expected4[] = "\0";
    test_decode(input4, output4, expected4, "Decode - Invalid UTF8 Input");

    printf("\n");
    printf("##############################################################################\n");

}

void test_strlen(unsigned char *string, int expected){

    int actual = my_utf8_strlen(string);
    printf("***TEST %s: string = %s, expected = %d, actual = %d\n", (actual == expected ? "PASSED" : "FAILED"), string, expected, actual);
}

void testall_strlen(void){

    printf("######################### Tests for my_utf8_strlen() #########################\n");
    int expected;

    // Hebrew Arieh
    unsigned char string1[] ="\u05d0\u05e8\u05d9\u05d4";
    expected = 4;
    test_strlen(string1, expected);

    // mix of ascii and utf8 characters
    unsigned char string2[] = "hello\xD7\x90\xD7\xA8\xD7\x99\xD7\x94" "goodbye" "\xF0\x90\x8D\x88";
    expected = 17;
    test_strlen(string2, expected);

    // invalid utf8 input string
    unsigned char string3[] = "\x10\x90\x8D\x88";
    expected = 0;
    test_strlen(string3, expected);

    // empty input string
    unsigned char string4[] = "";
    expected = 0;
    test_strlen(string4, expected);

    printf("\n");
    printf("##############################################################################\n");
}


void test_check(unsigned char *string, int expected){

    bool actual = my_utf8_check(string);
    printf("***TEST %s: string = %s, expected = %d, actual = %d\n", (actual == expected ? "PASSED" : "FAILED"), string, expected, actual);

}

void testall_check(void){

    printf("######################### Tests for my_utf8_check() #########################\n");
    bool expected;

    // Hebrew Arieh
    unsigned char string1[] = "\xD7\x90\xD7\xA8\xD7\x99\xD7\x94";
    expected = 0;
    test_check(string1, expected);

    // 1-byte valid utf8 input
    unsigned char string2[] = "\u0024";
    expected = 0;
    test_check(string2, expected);

    // 2-byte valid utf8 input
    unsigned char string3[] = "\u00a3";
    expected = 0;
    test_check(string3, expected);

    // 3-byte valid utf8 input
    unsigned char string4[] = "\u20ac";
    expected = 0;
    test_check(string4, expected);

    // 4-byte valid utf8 input
    unsigned char string5[] = "\U0001f602";
    expected = 0;
    test_check(string5, expected);

    // valid leading byte but invalid continuation byte
    unsigned char string6[] = "\xc2\xc3";
    expected = 1;
    test_check(string6, expected);

    // mix of ascii and utf8 characters - valid
    unsigned char string7[] = "\U0001f61f worried \U0001f616 confounded";
    expected = 0;
    test_check(string7, expected);

    // mostly valid input string, 1 invalid character
    unsigned char string8[] = "\U0001f61f worried \xc3\x01 \U0001f616 confounded";
    expected = 1;
    test_check(string8, expected);

    printf("\n");
    printf("##############################################################################\n");
}


void test_charat(unsigned char *string, int index, unsigned char *expected){

    bool passed = true;
    unsigned char* actual = my_utf8_charat(string, index);

    if (expected == NULL){
        printf("TEST %s: String = '%s', index = %d, expected = '%s', actual = '%s'\n", (actual == NULL ? "PASSED" : "FAILED"), string, index, expected, actual);
        return;
    }

    int i = 0;
    while (expected[i] != '\0' && actual[i] != '\0'){
        // make sure that the values at this location in both strings match
        if (expected[i] != actual[i]){
            passed = false;
            break;
        }
        i++;
    }
    if (expected[i] != actual[i])
        passed = false;

    printf("TEST %s: String = '%s', index = %d, expected = '%s', actual = '%s'\n", (passed ? "PASSED" : "FAILED"), string, index, expected, actual);
    free(actual);
}

void testall_charat(void){

    printf("######################### Tests for my_utf8_charat() #########################\n");

    int index;

    // simple ascii input
    unsigned char string1[] = "12345";
    index = 3;
    unsigned char expected1[] = "4";
    test_charat(string1, index, expected1);

    // simple utf8 input
    unsigned char string2[] = "\u05d7\u05e0\u05d4";
    index = 0;
    unsigned char expected2[] = "\u05d7";
    test_charat(string2, index, expected2);

    // empty input string
    unsigned char string3[] = "";
    index = 3;
    unsigned char *expected3 = NULL;
    test_charat(string3, index, expected3);

    // invalid utf8 input string
    unsigned char string4[] = "\x99";
    index = 3;
    unsigned char *expected4 = NULL;
    test_charat(string4, index, expected4);

    // invalid index
    unsigned char string5[] = "\U0001f613 \U0001f60f \U0001f62d";
    index = 6;
    unsigned char *expected5 = NULL;
    test_charat(string5, index, expected5);

    // ascii and utf8-character input string
    unsigned char string6[] = "\U0001f613 \U0001f60f \U0001f62d";
    index = 0;
    unsigned char expected6[] = "\U0001f613";
    test_charat(string6, index, expected6);

    index = 1;
    unsigned char expected7[] = " ";
    test_charat(string6, index, expected7);

    printf("\n");
    printf("##############################################################################\n");
}

void test_strcmp(unsigned char *string1, unsigned char *string2, bool expected){

    int actual = my_utf8_strcmp(string1, string2);
    printf("TEST %s: String1 = '%s', String2 = '%s', expected = '%d', actual = '%d'\n", (expected == actual ? "PASSED" : "FAILED"), string1, string2, expected, actual);

}

void testall_strcmp(void){

    printf("######################### Tests for my_utf8_strcmp() #########################\n");
    bool expected;

    // Hebrew Arieh
    unsigned char string1a[] ="\u05d0\u05e8\u05d9\u05d4";
    unsigned char string1b[] ="\u05d0\u05e8\u05d9\u05d4";
    expected = 0;
    test_strcmp(string1a, string1b, expected);

    // mix of ascii and utf8 characters - matching strings
    unsigned char string2a[] = "hello!\U0001f60a goodbye\U0001f622";
    unsigned char string2b[] = "hello!\U0001f60a goodbye\U0001f622";
    expected = 0;
    test_strcmp(string2a, string2b, expected);

    // matching invalid utf8 input strings
    unsigned char string3a[] = "\xd5\x14\x80";
    unsigned char string3b[] = "\xd5\x14\x80";
    expected = 1;
    test_strcmp(string3a, string3b, expected);

    // non-matching input strings
    unsigned char string4a[] = "\u05d7\u05e0\u05d4";
    unsigned char string4b[] = "Chana";
    expected = 1;
    test_strcmp(string4a, string4b, expected);

    // one empty input string, one non-empty input string
    unsigned char string5a[] = "\u05e9\u05dc\u05d5\u05dd";
    unsigned char string5b[] = "";
    expected = 1;
    test_strcmp(string5a, string5b, expected);

    // different length input strings but matching until the end of the shorter one
    unsigned char string6a[] = "\u05e9\u05dc\u05d5\u05dd";
    unsigned char string6b[] = "\u05e9\u05dc\u05d5\u05dd\u05e9\u05dc\u05d5\u05dd";
    expected = 1;
    test_strcmp(string6a, string6b, expected);

    printf("\n");
    printf("##############################################################################\n");
}


void test_strcat(unsigned char *dest, unsigned char *src, unsigned char *expected){

    // make a copy of the original destination string for future comparison
    unsigned char dest_copy[50];
    int i = 0;
    while(dest[i] != '\0'){
        dest_copy[i] = dest[i];
        i++;
    }
    dest_copy[i] = '\0';

    // now actually call the strcat() function
    bool success = my_utf8_strcat(dest, src);

    // check for invalid utf8 input
    if (success == 1){
        printf("Invalid input string passed to strcat()!\n");
    }

    // if valid inputs, check that concatenation was done correctly
    bool passed = true;

    // 'dest' contains the result of concatenation, so compare it against 'expected'
    int j = 0;
    while (dest[j] != '\0' && expected[j] != '\0'){
        if (dest[j] != expected[j]){
            passed = false;
            break;
        }
        j++;
    }
    if (dest[j] != expected[j])
        passed = false;
    printf("TEST %s: destination = %s, source = '%s', expected = '%s', actual = '%s'\n", (passed ? "PASSED" : "FAILED"), dest_copy, src, expected, dest);
}

void testall_strcat(void){
    printf("######################### Tests for my_utf8_strcat() #########################\n");

    // simple utf8 strings
    unsigned char dest1[] = "\u05d0\u05e0\u05d9";
    unsigned char src1[] = " \u05de\u05d0\u05de\u05d9\u05df";
    unsigned char expected1[] = "\u05d0\u05e0\u05d9 \u05de\u05d0\u05de\u05d9\u05df";
    test_strcat(dest1, src1, expected1);

    // mix of ascii and utf8
    unsigned char dest2[] = "\u05d0\u05e0\u05d9 ";
    unsigned char src2[] = "is 'I'";
    unsigned char expected2[] = "\u05d0\u05e0\u05d9 is 'I'";
    test_strcat(dest2, src2, expected2);

    // invalid utf8 input string
    unsigned char dest3[] = "\u05d0\xdd ";
    unsigned char src3[] = "hello";
    unsigned char expected3[] = "\u05d0\xdd ";
    test_strcat(dest3, src3, expected3);


    printf("\n");
    printf("##############################################################################\n");
}


void test_strreverse(unsigned char *input, unsigned char *output, unsigned char *expected){

    bool reverse_successful = my_utf8_strreverse(input, output);
    bool passed = true;

    // compare actual output against expected output
    int i = 0;
    while(output[i] != '\0' && expected[i] != '\0'){
        if (output[i] != expected[i]){
            passed = false;
            break;
        }
        i++;
    }
    if (expected[i] != output[i])
        passed = false;

    // display message
    if (passed){
        printf("***TEST PASSED: ");
        if (reverse_successful == 1)
            printf("Invalid utf8-encoded string %s dealt with correctly.\n", input);
        else
            printf("Reverse successful! input = '%s', actual = '%s',  expected = '%s'.\n", input, output, expected);
    }
    else{   // if test failed
        printf("***TEST FAILED: ");
        if (reverse_successful == 1)
            printf("Invalid utf8-encoded string %s dealt with incorrectly.\n", input);
        else
            printf("Reverse failed! input = '%s', actual = '%s',  expected = '%s'.\n", input, output, expected);
    }
}

void testall_strreverse(void){
    printf("######################### Tests for my_utf8_strreverse() #########################\n");

    // simple ascii string
    unsigned char input1[] = "hello";
    unsigned char output1[6];
    unsigned char expected1[] = "olleh";
    test_strreverse(input1, output1, expected1);

    // utf8 + ascii input
    unsigned char input2[] = "a \u2209 B";
    unsigned char output2[8];
    unsigned char expected2[] = "B \u2209 a";
    test_strreverse(input2, output2, expected2);

    // invalid utf8 input
    unsigned char input3[] = "abc \xe1\x12";
    unsigned char output3[8];
    unsigned char expected3[] = "\0";
    test_strreverse(input3, output3, expected3);

    // 1-character input string
    unsigned char input4[] = "\u05d1";
    unsigned char output4[8];
    unsigned char expected4[] = "\u05d1";
    test_strreverse(input4, output4, expected4);

    // empty input string
    unsigned char input5[] = "";
    unsigned char output5[8];
    unsigned char expected5[] = "";
    test_strreverse(input5, output5, expected5);

    printf("\n");
    printf("##############################################################################\n");
}