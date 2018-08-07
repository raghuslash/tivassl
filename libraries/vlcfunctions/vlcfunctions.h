#include<arduino.h>


char* interleaver(char* s, char interout[][8]);
char* manchester(char* s, char interout[][8], char manout[][16]);
char* foo(char* s, char manout[][16], char final[][56]);
void send_vlc_data(char* data, char final[][56], int);
