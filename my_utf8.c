#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// encode???
// add end-of-string flags
// what to do about leading ffffff in negative outputs??
// is strcmp() expecting 2 utf8 encoded strings?
// should output of decode() be in hex?


int my_utf8_encode(char *input, char *output);
int my_utf8_decode(char *input, char *output);
int my_utf8_strlen(char *string);
int my_utf8_check(char *string);
char *my_utf8_charat(char *string, int index);
int my_utf8_strcmp(char *string1, char *string2);
int my_utf8_strcat(char *dest, char *src);
int my_utf8_strreverse(char *input, char *output);
int my_utf8_charsize(unsigned char c);

/// TEST FUNCTIONS ///
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

//    char *a = "\u10FF";
//    printf("%c\n", *a);
//    char *output = "aa";
//    my_utf8_encode(input, output);

//    char *input = "אא";
    char input[] = "\xF0\x90\x8D\x88" "ldfkj";
    char output[20];
    my_utf8_decode(input, output);
//
//    char *string = "fds한Иאא";
    int length = my_utf8_strlen(input);
    printf("string length: %d\n", length);

    char *ans = my_utf8_charat(input, 5);
    printf("%s\n", ans);

//    char *string = "fds한Иאא";
//    int check = my_utf8_check(string);
//    printf("valid string? %d\n", check);


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

int my_utf8_encode(char *input, char *output)
{
    // convert input string into int value
    int length = strlen(input);
    printf("%d", length);

    // code points can be either 1, 2, or 3 bytes long
    printf("current char str val: %s\n", input);
    for (int i = 0; i < length; i++){
        // each time, advancing by 1 byte but not necessarily by one character
        // encode each character in the string

        printf("current char hex val: 0x%x\n", (unsigned char)input[i]);

//         if decimal value of this character is <= 127, it's an ascii
        if ((unsigned char)input[i] <= 127){
            output[i] = input[i];
            continue;
        }

//        else {
//
//        }
        // determine number of bytes that this
        int num_bytes = 0;
        if ((int)input[i] > 0 && (int)input[i] <= 0x007F)
            num_bytes = 1;

        else if ((int)input[i] >= 0x0080 && (int)input[i] <= 0x07FF)
            num_bytes = 2;

        else if ((int)input[i] >= 0x0800 && (int)input[i] <= 0xFFFF)
            num_bytes = 3;

        else if ((int)input[i] >= 0x10000 && (int)input[i] <= 0x10FFFF)
            num_bytes = 4;
    }
    return 1;
}

// put in the '\u's
int my_utf8_decode(char *input, char *output){

    // first, check that the input string is a valid UTF8 encoded string
    bool invalid = my_utf8_check(input);
    if (invalid)
        return 1;

    char *input_address = input;      // pointer to hold onto starting address of input string
    int cur_char_len = 0;   // number of bytes that a given character takes up
    int j = 0;              // output string index counter

    while (*input != '\0'){

        unsigned char cur_char = (unsigned char) *input;

        // if it's an ascii char, simply copy it into the output array
        if (cur_char >= 0x00 && cur_char < 0x80) {
            cur_char_len = 1;
            output[j++] = (signed char)cur_char;
        }
        else {
            // put in \u to signify that this is a unicode codepoint
            output[j++] = '\\';
            output[j++] = 'u';

            if (cur_char >= 0xc0 && cur_char < 0xe0) {
                cur_char_len = 2;   // this char is encoded in two bytes

                unsigned char a = cur_char - 0xc0;
                unsigned char b = ((unsigned char) input[1] - 0x80);

                output[j++] = (char) (a >> 2);
                output[j++] = (char) ((a << 6) + b);

            } else if (cur_char >= 0xe0 && cur_char < 0xf0) {
                cur_char_len = 3;

                // first byte of decoded string consists of last four bits in first byte of encoded string concatenated with its middle 4 bits
                unsigned char a = (cur_char - 0xe0) << 4;
                unsigned char b = ((unsigned char) input[1] - 0x80);
                unsigned char c = ((unsigned char) input[2] - 0x80);

                output[j++] = (char) (a + (b >> 2));
                output[j++] = (char) ((b << 6) + c);

            } else if (cur_char >= 0xf0 && cur_char < 0xf8){
                cur_char_len = 4;
                unsigned char a = (cur_char - 0xf0) << 2;
                unsigned char b = ((unsigned char) input[1] - 0x80);
                unsigned char c = ((unsigned char) input[2] - 0x80);
                unsigned char d = ((unsigned char) input[3] - 0x80);

                output[j++] = (char)(a + (b >> 4));
                output[j++] = (char)((b << 4) + (c >> 2));
                output[j++] = (char)((c << 6) + d);
            }
        }
        input += cur_char_len;
    }
    output[j] = '\0';   // end-of-string flag

    printf("input: %x%x%x%x\n", (unsigned char) input_address[0], (unsigned char) input_address[1],
           (unsigned char) input_address[2], (unsigned char) input_address[3]);
    printf("output: %02x%02x%02x\n", (unsigned char) output[0], (unsigned char) output[1],
           (unsigned char) output[2]);
    return 0;
}

// returns the number of utf8 encoded characters in the string
// returns 0 if string is empty or invalid utf8
int my_utf8_strlen(char *string)
{
    int invalid = my_utf8_check(string);
    if (invalid)
        return 0;

    int length = 0;
    int cur_char_len = 0;

    while (*string != '\0'){
        length++;
        unsigned char cur_char = (unsigned char)(*string);

        // determine how many bytes this character takes up
        cur_char_len = my_utf8_charsize(cur_char);

        // move to next character
        string += cur_char_len;
    }
    return length;
}


int my_utf8_check(char *string) {

    // first determine the number of bytes in the input string
    int length = 0;
    char *ptr = string; // new pointer so that we don't lose the original
    while (*ptr != '\0'){
        length++;
        ptr++;
    }

    // then loop through string again, checking whether each character is valid
    int cur_char_len = 0;
    for(int i = 0; i < length; i+= cur_char_len){
        unsigned int bytes_rem = length - i - 1;            // number of bytes that remain in the string after this one
        unsigned char cur_char = (unsigned char) string[i];   // cast character to an unsigned char for easy hex comparison
        unsigned char next_char;
        unsigned char next_next_char;
        unsigned char next_next_next_char;

        // Range #1 (less than 0x08 --> ascii character
        if (cur_char < 0x80) {
            cur_char_len = 1;
        }

            // if not an ascii character and no bytes follow, it's an invalid utf8 string
        else if (cur_char >= 0x80 && bytes_rem == 0)
            return 1;

            // Range #2
        else if (cur_char >= 0xc0 && cur_char < 0xe0) {

            next_char = (unsigned char) string[i+1];
            if (!(next_char >= 0x80 && next_char < 0xc0))
                return 1;

            cur_char_len = 2;
        }

            // Range #3
        else if (cur_char >= 0xe0 && cur_char < 0xf0) {
            if (bytes_rem < 2)
                return 1;

            // otherwise...
            next_char = (unsigned char) string[i+1];
            next_next_char = (unsigned char) string[i+2];
            if (!(next_char >= 0x80 && next_char < 0xc0 && next_next_char >= 0x80 && next_next_char < 0xc0))
                return 1;

            cur_char_len = 3;
        }

            // Range #4
        else if (cur_char >= 0xf0 && cur_char < 0xf8) {
            if (bytes_rem < 3)
                return 1;

            // otherwise...
            next_char = (unsigned char) string[i+1];
            next_next_char = (unsigned char) string[i+2];
            next_next_next_char = (unsigned char) string[i+3];
            if (!(next_char >= 0x80 && next_char < 0xc0 && next_next_char >= 0x80 && next_next_char < 0xc0 &&
                  next_next_next_char >= 0x80 && next_next_next_char < 0xc0))
                return 1;

            cur_char_len = 4;
        }
        else
            return 1;
    }
    return 0;
}

// returns the utf-encoded character at the given index
char *my_utf8_charat(char *string, int index){

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
        unsigned char cur_char = (unsigned char)(*string);

        // determine how many bytes this character takes up
        cur_char_len = my_utf8_charsize(cur_char);

        // move to next character only if we're not already at the desired character
        if (cur_pos < index)
            string += cur_char_len;
        cur_pos++;
    }

    // now, the first element in the string is the desired character (but it might be spread over multiple memory locations)
    // build up answer string
    char* ans = (char*) malloc(cur_char_len);
    int i;
    for (i = 0; i < cur_char_len; i++)
        ans[i] = string[i];
    ans[i] = '\0';  // end-of-string flag

    return ans;

}

// assuming string1 and string2 are utf8 encoded strings ??????
int my_utf8_strcmp(char *string1, char *string2) {

    while (*string1 == *string2 && *string1 != '\0') {
        string1++;
        string2++;
    }

    // if here, either the strings are not matching or we've reached the end of string1
    // if we've also reached the end of string2, string1 == string2
    if (*string2 == '\0')
        return 0;
    return 1;
}

// concatenates source string to the end of the destination string if both are valid utf8 strings
int my_utf8_strcat(char *dest, char *src){

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
int my_utf8_strreverse(char *input, char *output){

    // determine length of input (total number of bytes)
    int length = 0;
    char *input_ptr = input;
    while (*input_ptr != '\0'){
        length++;
        input_ptr++;
    }

    // for each char, determine many bytes it takes up --> put it at the rightmost position (minus the char size+1) in output
    int cur_char_len = 0;
    int i;
    for (i = 0; i < length; i += cur_char_len){

        unsigned char cur_char = (unsigned char)(input[i]);

        // determine how many bytes this character takes up
        cur_char_len = my_utf8_charsize(cur_char);
    }

}

////////////////////////////////
/////// TEST FUNCTIONS /////////
////////////////////////////////

void test_decode(void){

    // test Hebrew Arieh
    char input[] = "\xD7\x90\xD7\xA8\xD7\x99\xD7\x94";
    char output[20];
    my_utf8_decode(input, output);

    assert((unsigned char)output[0] == '\\');
    assert((unsigned char)output[1] == 'u');
    assert((unsigned char)output[2] == 0x05);
    assert((unsigned char)output[3] == 0xd0);

    assert((unsigned char)output[4] == '\\');
    assert((unsigned char)output[5] == 'u');
    assert((unsigned char)output[6] == 0x05);
    assert((unsigned char)output[7] == 0xe8);

    assert((unsigned char)output[8] == '\\');
    assert((unsigned char)output[9] == 'u');
    assert((unsigned char)output[10] == 0x05);
    assert((unsigned char)output[11] == 0xd9);

    assert((unsigned char)output[12] == '\\');
    assert((unsigned char)output[13] == 'u');
    assert((unsigned char)output[14] == 0x05);
    assert((unsigned char)output[15] == 0xd4);
    assert(output[16] == '\0');
}

void test_decode_long_input(void){
    char input[] = "abcdefg\xD7\x90" "12345678" "\xD7\xA8" "\xF0\x90\x8D\x88" "\xD7\x99\xD7\x94";
    char output[40];

    my_utf8_decode(input, output);
    printf("%s\n", output);

    char *ans = "abcdefg" "\u05d0" "12345678" "\\u05e8" "\\u10348" "\\u05D9\\u05D4";
    printf("%s\n", ans);

//    assert(output == ans);

    assert((unsigned char)output[0] == 'a');
    assert((unsigned char)output[1] == 'b');
    assert((unsigned char)output[2] == 'c');
    assert((unsigned char)output[3] == 'd');
    assert((unsigned char)output[4] == 'e');
    assert((unsigned char)output[5] == 'f');
    assert((unsigned char)output[6] == 'g');

    assert((unsigned char)output[7] == '\\');
    assert((unsigned char)output[8] == 'u');
    assert((unsigned char)output[9] == 0x05);
    assert((unsigned char)output[10] == 0xd0);
//    assert((unsigned char)output[11] == '\\');
//    assert((unsigned char)output[7] == '\\');
}

void test_decode_empty_string(void){
    char input[] = "";
    char output[20];
    my_utf8_decode(input, output);

    // output should consist of only the end-of-string flag
    assert(output[0] == '\0');
}

void test_decode_invalid_input(void){
    char input[] = "\x01\xD0\x02";  // INVALID utf8 string
    char output[20];

    assert(my_utf8_decode(input, output) == 1);
}

/// tests for strlen()
// test strlen() with simple input
void test_strlen_simple(void){
    char input[] = "\xD7\x90\xD7\xA8\xD7\x99\xD7\x94";
    assert(my_utf8_strlen(input) == 4);
}

// test strlen() with input that's a mix of ascii and utf8 encoded characters
void test_strlen_ascii_and_utf8(void){
    char input[] = "hello\xD7\x90\xD7\xA8\xD7\x99\xD7\x94" "goodbye" "\xF0\x90\x8D\x88";
    assert(my_utf8_strlen(input) == 17);
}

void test_strlen_invalid_input(void){
    char input[] = "\x10\x90\x8D\x88";
    assert(my_utf8_strlen(input) == 0);
}
void test_strlen_empty_string(void){
    char input[] = "";
    assert(my_utf8_strlen(input) == 0);
}

/// tests for check()
// test that check() correctly validates valid utf8 encoded strings
void test_check_valid(void){
    char input1[] = "\xD7\x90\xD7\xA8\xD7\x99\xD7\x94";
    assert(my_utf8_check(input1) == 0);

    char input2[] = "\xE2\x82\xAC";
    assert(my_utf8_check(input2) == 0);

    char input3[] = "\xC2\xA3";
    assert(my_utf8_check(input3) == 0);

    char input4[] = "\xC2\xA3\x24";
    assert(my_utf8_check(input4) == 0);
}

// test that check() correctly invalidates invalid utf8 encoded strings
void test_check_invalid(void){
    char input1[] = "\x90\xD7\xA8\xD7\x99\xD7\x94";
    assert(my_utf8_check(input1) == 1);

    char input2[] = "\xE2\x82";
    assert(my_utf8_check(input2) == 1);

    char input3[] = "\xC2";
    assert(my_utf8_check(input3) == 1);

    char input4[] = "\xA3\x24";
    assert(my_utf8_check(input4) == 1);
}

// test check() with input that's a mix of ascii and utf8 encoded characters
void test_check_ascii_and_utf8(void){

    char input1[] = "123\x90" "101010" "\xD7\xA8\xD7\x99\xD7\x94";  // invalid utf8
    assert(my_utf8_check(input1) == 1);

    char input2[] = "computer\xE2\x82\xACorganization";             // valid utf8
    assert(my_utf8_check(input2) == 0);

    char input3[] = "\xE2\x82\xAC" "is so much fun" "\xC2";         // some valid chars, some invalid --> invalid string
    assert(my_utf8_check(input3) == 1);
}

/// tests for charat()

// test charat() with simple valid input
void test_charat_simple(void){
    char input1[] = "12345";
    char *output1 = my_utf8_charat(input1, 3);

    assert(*output1 == '4');

    char input2[] = "\xD7\x97\xD7\xA0\xD7\x94";     // 3 Hebrew characters total
    char *output2 = my_utf8_charat(input2, 1);

    assert(output2[0] == '\xD7');
    assert(output2[1] == '\xA0');
}

// test charat() with empty input string
void test_charat_empty_string(void){
    char input[] = "";
    char *output = my_utf8_charat(input, 3);
    assert(output == NULL);
}

void test_charat_invalid_input(void){
    char input[] = "\x90\xD8\x01";      // invalid utf8 string
    char *output = my_utf8_charat(input, 0);
    assert(output == NULL);
}

void test_charat_ascii_and_utf8(void){
    char input[] = "hello\xE2\x82\xACgoodbye";

    // ascii character before any utf8 characters
    char *output1 = my_utf8_charat(input, 2);
    assert(*output1 == 'l');

    // utf8 character
    char *output2 = my_utf8_charat(input, 5);
    assert(output2[0] == '\xE2');
    assert(output2[1] == '\x82');
    assert(output2[2] == '\xAC');

    // ascii character following a utf8 character
    char *output3 = my_utf8_charat(input, 6);
    assert(*output3 == 'g');
}

/// tests for strcmp()
void test_strcmp_simple(void){}
void test_strcmp_ascii_and_utf8(void){}
void test_strcmp_invalid_input(void){}
void test_strcmp_matching(void){}
void test_strcmp_dif_lengths(void){}
void test_strcmp_same_lengths(void){}
void test_strcmp_empty_string(void){}


void test_strcat_simple(void){

    char source[] = "that is the question";
    char dest[] = "to utf8 or not \xED\x95\x9C ";

    // assert that there were no errors
    assert(my_utf8_strcat(dest, source) == 0);

    // assert that the correct result was obtained
    char ans[] = "to utf8 or not \xED\x95\x9C that is the question";
    int i = 0;
    while(ans[i] != '\0') {
        assert(dest[i] == ans[i]);
        i++;
    }
}


