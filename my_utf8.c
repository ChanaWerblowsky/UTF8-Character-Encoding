#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// add end-of-string flags
// test functions
// add informative error messages
// is it ok if depends on hex being lowercase? -- update to allow for uppercase


int my_utf8_encode(unsigned char *input, unsigned char *output);
int my_utf8_decode(unsigned char *input, unsigned char *output);
int my_utf8_strlen(unsigned char *string);
int my_utf8_check(unsigned char *string);
unsigned char *my_utf8_charat(unsigned char *string, int index);
int my_utf8_strcmp(unsigned char *string1, unsigned char *string2);
int my_utf8_strcat(unsigned char *dest, unsigned char *src);
int my_utf8_strreverse(unsigned char *input, unsigned char *output);
int my_utf8_charsize(unsigned char c);
unsigned char hexdigit_to_ascii(int h);
int asciichar_to_hex(unsigned char c);
unsigned char *hex_to_ascii(int h);
int ascii_to_hex(unsigned char *c);

/// TEST FUNCTIONS ///


void test_encode(unsigned char *input, unsigned char *output, unsigned char *expected, char *test_name);
void testall_encode();
void test_encode_simple(void);
void test_encode_long_char(void);
void test_encode_invalid_input(void);

// tests for decode()
void test_decode(void);
void test_decode_long_input(void);
void test_decode_empty_string(void);
void test_decode_invalid_input(void);

// tests for strlen()
void test_strlen_simple(void);
void test_strlen_ascii_and_utf8(void);
void test_strlen_invalid_input(void);
void test_strlen_empty_string(void);

// tests for check()
void test_check_valid(void);
void test_check_invalid(void);
void test_check_ascii_and_utf8(void);

// tests for charat()
void test_charat_simple(void);
void test_charat_empty_string(void);
void test_charat_invalid_input(void);
void test_charat_ascii_and_utf8(void);

// tests for strcmp()
void test_strcmp_simple(void);
void test_strcmp_ascii_and_utf8(void);
void test_strcmp_invalid_input(void);
void test_strcmp_matching(void);
void test_strcmp_dif_lengths(void);
void test_strcmp_same_lengths(void);
void test_strcmp_empty_string(void);

void test_strcat_simple(void);

int main() {

//    char *input = "אא";
//    unsigned char input[] = "\xF0\x90\x8D\x88" "ldfkj";
//    unsigned char output[20];
//    my_utf8_decode(input, output);


//    char *string = "fds한Иאא";
//    int check = my_utf8_check(string);

    unsigned char input1[] = "a\xF0\x90\x8D\x88";
    unsigned char output1[10];
    my_utf8_strreverse(input1, output1);

    testall_encode();

    // test decode()
    test_decode();
    test_decode_long_input();
    test_decode_invalid_input();
    test_decode_empty_string();

    // test strlen()
    test_strlen_simple();
    test_strlen_ascii_and_utf8();
    test_strlen_invalid_input();
    test_strlen_empty_string();

    // test check()
    test_check_valid();
    test_check_invalid();
    test_check_ascii_and_utf8();

    // test charat()
    test_charat_simple();
    test_charat_empty_string();
    test_charat_invalid_input();
    test_charat_ascii_and_utf8();

    // test strcmp()
    test_strcmp_simple();
    test_strcmp_ascii_and_utf8();
    test_strcmp_invalid_input();
    test_strcmp_matching();
    test_strcmp_dif_lengths();
    test_strcmp_same_lengths();
    test_strcmp_empty_string();

    test_strcat_simple();

    return 0;
}

/// Helper functions

// converts a hex digit to its ascii representation
unsigned char hexdigit_to_ascii(int h){

    unsigned char ans = '\0';

    // if h is between 0x00 and 0x09, add 0x30 to get its ascii representation
    if (h >= 0x00 && h <= 0x09)
        ans = (unsigned char)(h + 0x30);

    // if h is between 0x0a and 0x0f, add 0x57 to get its (lowercase) ascii representation
    else if (h >= 0x0a && h <= 0x0f)
        ans = (unsigned char)(h + 0x57);

    return ans;
}

// given a 1 byte hex value, convert it to 2 ascii characters
unsigned char *hex_to_ascii(int h){

    // isolate each of the two hex digits
    int left_digit1 = h >> 4;
    int right_digit1 = h - (left_digit1 <<4);

    // and convert them to ascii
    unsigned char c1 = hexdigit_to_ascii(left_digit1);
    unsigned char c2 = hexdigit_to_ascii(right_digit1);

    // check for errors
    if (c1 == '\0' || c2 == '\0')
        return NULL;

    // save result in array
    unsigned char *output = (unsigned char*) malloc(2);
    output[0] = c1;
    output[1] = c2;
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


// given a series of ascii characters, converts them to 'matching' hex int
int ascii_to_hex(unsigned char *c){

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
    int ans = 0;
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

// encodes the given unicode codepoint in UTF8
// if an invalid character or codepoint is encountered, the output string is terminated there
int my_utf8_encode(unsigned char *input, unsigned char *output)
{
    bool success = true;
    int j = 0;    // index in output string
    while (*input != '\0'){

        if (input[0] == '\\') {

            int num_digits = 0;
            if (input[1] == 'u')
                num_digits = 4;

            else if (input[1] == 'U')
                num_digits = 8;

            // if we're at a \u or \U:
            if (num_digits != 0) {

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
                int codepoint = ascii_to_hex(codepoint_str);

                // if invalid character following the \u, terminate the output string
                if (codepoint == -1){
                    success = false;
                    printf("Invalid character following \\%c!\n", input[1]);
                    break;
                }

                if (num_digits == 4){
                    // determine in which range this codepoint lies; apply correct encoding algorithm:
                    int left_byte = codepoint >> 8;     // isolate the two hex bytes
                    int right_byte = codepoint & 0x00ff;

                    // if less than U+00a0, only U+0024, U+0040, and U+0060 are valid
                    if (codepoint >= 0x0000 && codepoint <= 0x007f) {
                        if (codepoint != 0x0024 && codepoint != 0x0040 && codepoint != 0x0060){
                            success = false;
                            printf("Invalid universal character \\u%s - less than 0xa0\n", codepoint_str);
                            break;
                        }
                        output[j++] = right_byte;
                    }
                    else if (codepoint >= 0x0080 && codepoint <= 0x07ff) {

                        if (codepoint < 0xa0){  // if within invalid range
                            success = false;
                            printf("Invalid universal character \\u%s - less than 0xa0\n", codepoint_str);
                            break;
                        }
                        output[j++] = 0xc0 + ((left_byte << 2) + (right_byte >> 6));
                        output[j++] = 0x80 + (right_byte & 0x3f);
                    }
                    else if (codepoint >= 0x0800 && codepoint <= 0xffff) {  // if within invalid range
                        if (codepoint >= 0xd800 && codepoint <= 0xdfff){
                            success = false;
                            printf("Invalid universal character \\u%s - within range 0xd800-0xdfff\n", codepoint_str);
                            break;
                        }
                        output[j++] = 0xe0 + (left_byte >> 4);
                        output[j++] = 0x80 + ((left_byte & 0x0f) << 2) + (right_byte >> 6);
                        output[j++] = 0x80 + (right_byte & 0x3f);
                    }
                }

                else {  // num_digits == 8
                    int left_byte = codepoint >> 16;
                    int mid_byte = (codepoint >> 8) & 0x0000ff;
                    int right_byte = codepoint & 0x0000ff;

                    if (codepoint >= 0x10000 && codepoint <= 0x10ffff) {
                        output[j++] = 0xf0 + (left_byte >> 2);
                        output[j++] = 0x80 + ((left_byte << 4) + (mid_byte >> 4));
                        output[j++] = 0x80 + ((mid_byte << 2) & 0x3f) + (right_byte >> 6);
                        output[j++] = 0x80 + (right_byte & 0x3f);
                    }

                    // value of codepoint is above the largest unicode codepoint --> terminate the output string
                    else{
                        success = false;
                        printf("Invalid universal character \\u%s - greater than 0x10FFFF\n", codepoint_str);
                        break;
                    }
                }

                // move to next character in input string
                input += num_digits+2;
            }
        }
        // if here we're not at a '\u' or '\U'
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

// put in the '\u's
int my_utf8_decode(unsigned char *input, unsigned char *output){

    // first, check that the input string is a valid UTF8 encoded string
    bool invalid = my_utf8_check(input);
    if (invalid)
        return 1;

    unsigned char *input_address = input;      // pointer to hold onto starting address of input string
    int cur_char_len = 0;   // number of bytes that a given character takes up
    int j = 0;              // output string index counter

    while (*input != '\0'){

        // if it's an ascii char, simply copy it into the output array
        if (input[0] >= 0x00 && input[0] < 0x80) {
            cur_char_len = 1;
            output[j++] = input[0];
        }
        else {
            // put in \u to signify that this is a unicode codepoint
            output[j++] = '\\';

            if (input[0] >= 0xc0 && input[0] < 0xe0) {
                cur_char_len = 2;   // this char is encoded in two bytes

                unsigned char a = input[0] - 0xc0;
                unsigned char b = input[1] - 0x80;

                // result in hex: 2 hex values where each consists of 2 hex digits
                int hex_byte1 = a >> 2;
                int hex_byte2 = ((a << 6) + b) & 0x00ff;

                // convert the 2 hex bytes into 4 ascii characters
                unsigned char *ascii1 = hex_to_ascii(hex_byte1);
                unsigned char *ascii2 = hex_to_ascii(hex_byte2);

                output[j++] = 'u';
                output[j++] = ascii1[0];
                output[j++] = ascii1[1];
                output[j++] = ascii2[0];
                output[j++] = ascii2[1];

                free(ascii1);
                free(ascii2);

            } else if (input[0] >= 0xe0 && input[0] < 0xf0) {
                cur_char_len = 3;

                // first byte of decoded string consists of last four bits in first byte of encoded string concatenated with its middle 4 bits
                int a = (input[0] - 0xe0) << 4;
                int b = (input[1] - 0x80);
                int c = (input[2] - 0x80);

                int hex_byte1 = a + (b >> 2);
                int hex_byte2 = ((b << 6) + c) & 0x00ff;

                // convert the 2 hex bytes into 4 ascii characters
                unsigned char *ascii1 = hex_to_ascii(hex_byte1);
                unsigned char *ascii2 = hex_to_ascii(hex_byte2);

                output[j++] = 'u';
                output[j++] = ascii1[0];
                output[j++] = ascii1[1];
                output[j++] = ascii2[0];
                output[j++] = ascii2[1];

                free(ascii1);
                free(ascii2);

            } else if (input[0] >= 0xf0 && input[0] < 0xf8){
                cur_char_len = 4;
                int a = (input[0] - 0xf0) << 2;
                int b = (input[1] - 0x80);
                int c = (input[2] - 0x80);
                int d = (input[3] - 0x80);

                int hex_byte1 = a + (b >> 4);
                int hex_byte2 = ((b << 4) + (c >> 2)) & 0x00ff;
                int hex_byte3 = ((c << 6) + d) & 0x00ff;

                // convert the 3 hex bytes into 6 ascii characters
                unsigned char *ascii1 = hex_to_ascii(hex_byte1);
                unsigned char *ascii2 = hex_to_ascii(hex_byte2);
                unsigned char *ascii3 = hex_to_ascii(hex_byte3);

                output[j++] = 'U';
                output[j++] = '0';
                output[j++] = '0';
                output[j++] = ascii1[0];
                output[j++] = ascii1[1];
                output[j++] = ascii2[0];
                output[j++] = ascii2[1];
                output[j++] = ascii3[0];
                output[j++] = ascii3[1];

                free(ascii1);
                free(ascii2);
                free(ascii3);
            }
        }
        input += cur_char_len;
    }
    output[j] = '\0';   // end-of-string flag
    return 0;
}

// returns the number of utf8 encoded characters in the string
// returns 0 if string is empty or invalid utf8
int my_utf8_strlen(unsigned char *string)
{
    int invalid = my_utf8_check(string);
    if (invalid)
        return 0;

    int length = 0;
    int cur_char_len = 0;

    while (*string != '\0'){
        length++;

        // determine how many bytes this character takes up
        cur_char_len = my_utf8_charsize(*string);
        string += cur_char_len;     // and move to the next character
    }
    return length;
}

// returns 0 if the input string is a valid utf8-encoded string, and 1 otherwise
int my_utf8_check(unsigned char *string) {

    // first determine the number of bytes in the input string
    int length = 0;
    unsigned char *ptr = string; // new pointer so that we don't lose the original
    while (*ptr != '\0'){
        length++;
        ptr++;
    }

    // then loop through string again, checking whether each character is valid
    int cur_char_len = 0;
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

// returns the utf-encoded character at the given index
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
    // build up answer string
    unsigned char *ans = (unsigned char*) malloc(cur_char_len);
    int i;
    for (i = 0; i < cur_char_len; i++)
        ans[i] = string[i];
    ans[i] = '\0';  // end-of-string flag

    return ans;
}

// returns 0 if the utf8-encoded strings string1 and string2 are identical, and 1 otherwise
int my_utf8_strcmp(unsigned char *string1, unsigned char *string2) {

    while (*string1 != '\0' && *string1 == *string2) {
        string1++;
        string2++;
    }

    // if here, either we've found a non-matching character or we've reached the end of string1
    if (*string2 == '\0')   // if we've also reached the end of string2, string1 == string2
        return 0;

    return 1;               // otherwise, string1 != string2
}

// concatenates source string to the end of the destination string if both are valid utf8 strings
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

    return 0;
}

// reverses a utf8 encoded string
int my_utf8_strreverse(unsigned char *input, unsigned char *output){

    int invalid = my_utf8_check(input);
    if (invalid)
        return 1;

    // determine length of input (total number of bytes)
    int length = 0;
    unsigned char *input_ptr = input;
    while (*input_ptr != '\0'){
        length++;
        input_ptr++;
    }
    // for each char, determine many bytes it takes up and put it at the leftmost available position in output
    int cur_char_len = 0;
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
    output[length] = '\0';
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// TEST FUNCTIONS //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

void test_encode(unsigned char *input, unsigned char *output, unsigned char *expected, char *test_name){
    bool encode_successful = my_utf8_encode(input, output);  // call the encode() function

    // compare output against correct answer
    int i = 0;
    bool passed = true;
    while(output[i] != '\0' && expected[i] != '\0'){
        if (output[i] != expected[i]){
            printf("%s: TEST FAILED! ", test_name);
            if (encode_successful == 1)
                printf("Invalid input %s dealt with incorrectly.\n", input);
            else
                printf("Failed to properly encode the string %s! Actual: %s, Expected: %s\n", input, output, expected);

            passed = false;
            break;
        }
        i++;
    }
    if (passed){
        printf("%s: TEST PASSED. ", test_name);
        if (encode_successful == 1)
            printf("Invalid input %s dealt with correctly.\n", input);
        else
            printf("Successfully encoded the string %s as %s.\n", input, output);
    }
}

// tests encode() with various test cases
void testall_encode(){

    printf("######################### Tests for my_utf8_encode() #########################\n");

    // 1. simple input
    unsigned char input1[] = "\\u05d0\\u05e8\\u05d9\\u05d4";
    unsigned char output1[10];
    unsigned char expected1[] = "\u05d0\u05e8\u05d9\u05d4";
    test_encode(input1, output1, expected1, "Encode - simple input");
    printf("\n");

    // 2. Range 1 codepoint
    unsigned char input2[] = "\\u0024";
    unsigned char output2[10];
    unsigned char expected2[] = "\u0024";
    test_encode(input2, output2, expected2, "Encode - Range 1 codepoint");
    printf("\n");


    // 3. Range 2 codepoint
    unsigned char input3[] = "\\u00A3";
    unsigned char output3[10];
    unsigned char expected3[] = "\u00A3";
    test_encode(input3, output3, expected3, "Encode - Range 2 codepoint");
    printf("\n");


    // 4. Range 3 codepoint
    unsigned char input4[] = "\\ud55c";
    unsigned char output4[10];
    unsigned char expected4[] = "\ud55c";
    test_encode(input4, output4, expected4, "Encode - Range 3 codepoint");
    printf("\n");


    // 5. Range 4 codepoint (8 digits)
    unsigned char input5[] = "\\U00010348";
    unsigned char output5[10];
    unsigned char expected5[] = "\xf0\x90\x8d\x88";
    test_encode(input5, output5, expected5, "Encode - Range 4 codepoint");
    printf("\n");


    // 6. invalid hex digit in codepoint
    unsigned char input6[] = "\\ug0187";
    unsigned char output6[10];
    unsigned char expected6[] = "\0";
    test_encode(input6, output6, expected6, "Encode - non-hex in codepoint");
    printf("\n");


    // 7. incomplete codepoint at the start of the input string
    unsigned char input7[] = "\\u018";
    unsigned char output7[10];
    unsigned char expected7[] = "\0";
    test_encode(input7, output7, expected7, "Encode - incomplete codepoint at beginning");
    printf("\n");


    // 8. incomplete codepoint at the end of the input string
    unsigned char input8[] = "\\uabcd\\u18";
    unsigned char output8[10];
    unsigned char expected8[] = "\uabcd";
    test_encode(input8, output8, expected8, "Encode - incomplete codepoint at end");
    printf("\n");


    // 9. missing '\u' before non-ascii character
    unsigned char input9[] = "€018";
    unsigned char output9[10];
    unsigned char expected9[] = "\0";
    test_encode(input9, output9, expected9, "Encode - missing '\\u'");
    printf("\n");

    // 10. within protected range (less than 0xa0 and not one of the exceptions)
    unsigned char input10[] = "\\u0035";
    unsigned char output10[10];
    unsigned char expected10[] = "\0";
    test_encode(input10, output10, expected10, "Encode - invalid codepoint");
    printf("\n");

    // 11. within protected range (between 0xd800 and 0xdfff)

    // 12. too large

    printf("##############################################################################");
}


void test_decode(void){

    // test Hebrew Arieh
    unsigned char input[] = "\xd7\x90\xd7\xa8\xd7\x99\xd7\x94";
    unsigned char output[30];
    my_utf8_decode(input, output);

    // correct answer
    unsigned char ans[] = "\\u05d0\\u05e8\\u05d9\\u05d4";

    // compare output to correct answer
    int i = 0;
    while (ans[i] != '\0'){
        assert(output[i] == ans[i]);
        i++;
    }
}

void test_decode_long_input(void){
    unsigned char input[] = "abcdefg\xd7\x90" "12345678" "\xd7\xa8" "\xf0\x90\x8d\x88" "\xd7\x99\xd7\x94";
    unsigned char output[60];

    my_utf8_decode(input, output);  // call decode()

    // correct answer
    char ans[] = "abcdefg\\u05d012345678\\u05e8\\U00010348\\u05d9\\u05d4";

    // compare output to correct answer
    int i = 0;
    while (ans[i] != '\0'){
        assert(output[i] == ans[i]);
        i++;
    }
}

void test_decode_empty_string(void){
    unsigned char input[] = "";
    unsigned char output[20];
    my_utf8_decode(input, output);

    // output should consist of only the end-of-string flag
    assert(output[0] == '\0');
}

void test_decode_invalid_input(void){
    unsigned char input[] = "\x01\xD0\x02";  // INVALID utf8 string
    unsigned char output[20];

    assert(my_utf8_decode(input, output) == 1);
}

/// tests for strlen()
// test strlen() with simple input
void test_strlen_simple(void){
    unsigned char input[] = "\xD7\x90\xD7\xA8\xD7\x99\xD7\x94";
    assert(my_utf8_strlen(input) == 4);
}

// test strlen() with input that's a mix of ascii and utf8 encoded characters
void test_strlen_ascii_and_utf8(void){
    unsigned char input[] = "hello\xD7\x90\xD7\xA8\xD7\x99\xD7\x94" "goodbye" "\xF0\x90\x8D\x88";
    assert(my_utf8_strlen(input) == 17);
}

void test_strlen_invalid_input(void){
    unsigned char input[] = "\x10\x90\x8D\x88";
    assert(my_utf8_strlen(input) == 0);
}
void test_strlen_empty_string(void){
    unsigned char input[] = "";
    assert(my_utf8_strlen(input) == 0);
}

/// tests for check()
// test that check() correctly validates valid utf8 encoded strings
void test_check_valid(void){
    unsigned char input1[] = "\xD7\x90\xD7\xA8\xD7\x99\xD7\x94";
    assert(my_utf8_check(input1) == 0);

    unsigned char input2[] = "\xE2\x82\xAC";
    assert(my_utf8_check(input2) == 0);

    unsigned char input3[] = "\xC2\xA3";
    assert(my_utf8_check(input3) == 0);

    unsigned char input4[] = "\xC2\xA3\x24";
    assert(my_utf8_check(input4) == 0);
}

// test that check() correctly invalidates invalid utf8 encoded strings
void test_check_invalid(void){
    unsigned char input1[] = "\x90\xD7\xA8\xD7\x99\xD7\x94";
    assert(my_utf8_check(input1) == 1);

    unsigned char input2[] = "\xE2\x82";
    assert(my_utf8_check(input2) == 1);

    unsigned char input3[] = "\xC2";
    assert(my_utf8_check(input3) == 1);

    unsigned char input4[] = "\xA3\x24";
    assert(my_utf8_check(input4) == 1);
}

// test check() with input that's a mix of ascii and utf8 encoded characters
void test_check_ascii_and_utf8(void){

    unsigned char input1[] = "123\x90" "101010" "\xD7\xA8\xD7\x99\xD7\x94";  // invalid utf8
    assert(my_utf8_check(input1) == 1);

    unsigned char input2[] = "computer\xE2\x82\xACorganization";             // valid utf8
    assert(my_utf8_check(input2) == 0);

    unsigned char input3[] = "\xE2\x82\xAC" "is so much fun" "\xC2";         // some valid chars, some invalid --> invalid string
    assert(my_utf8_check(input3) == 1);
}

/// tests for charat()

// test charat() with simple valid input
void test_charat_simple(void){
    unsigned char input1[] = "12345";
    unsigned char *output1 = my_utf8_charat(input1, 3);

    assert(output1[0] == '4');
    free(output1);

    unsigned char input2[] = "\xD7\x97\xD7\xA0\xD7\x94";     // 3 Hebrew characters total
    unsigned char *output2 = my_utf8_charat(input2, 1);

    assert(output2[0] == 0xD7);
    assert(output2[1] == 0xA0);
    free(output2);
}

// test charat() with empty input string
void test_charat_empty_string(void){
    unsigned char input[] = "";
    unsigned char *output = my_utf8_charat(input, 3);
    assert(output == NULL);
    free(output);
}

void test_charat_invalid_input(void){
    unsigned char input[] = "\x90\xD8\x01";      // invalid utf8 string
    unsigned char *output = my_utf8_charat(input, 0);
    assert(output == NULL);
    free(output);
}

void test_charat_ascii_and_utf8(void){
    unsigned char input[] = "hello\xE2\x82\xACgoodbye";

    // ascii character before any utf8 characters
    unsigned char *output1 = my_utf8_charat(input, 2);
    assert(*output1 == 'l');

    // utf8 character
    unsigned char *output2 = my_utf8_charat(input, 5);
    assert(output2[0] == (unsigned int) 0xE2);
    assert(output2[1] == (unsigned int) 0x82);
    assert(output2[2] == (unsigned int) 0xAC);

    // ascii character following a utf8 character
    unsigned char *output3 = my_utf8_charat(input, 6);
    assert(*output3 == 'g');

    free(output1);
    free(output2);
    free(output3);
}

/// tests for strcmp()
void test_strcmp_simple(void){
    unsigned char string1[] = "ab\u1234";
    unsigned char string2[] = "abc\u1234";
    int result = my_utf8_strcmp(string1, string2);
    assert(result == 1);
}

void test_strcmp_ascii_and_utf8(void){}
void test_strcmp_invalid_input(void){}
void test_strcmp_matching(void){}
void test_strcmp_dif_lengths(void){}
void test_strcmp_same_lengths(void){}
void test_strcmp_empty_string(void){}


void test_strcat_simple(void){

    unsigned char source[] = "that is the question";
    unsigned char dest[] = "to utf8 or not \xED\x95\x9C ";

    // assert that there were no errors
    assert(my_utf8_strcat(dest, source) == 0);

    // assert that the correct result was obtained
    unsigned char ans[] = "to utf8 or not \xED\x95\x9C that is the question";
    int i = 0;
    while(ans[i] != '\0') {
        assert(dest[i] == ans[i]);
        i++;
    }
}


