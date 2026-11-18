#include <stdio.h>
#include <string.h>
#include <locale.h>
#define BUFFER_SIZE 10
#define NODE_SIZE 32
#define LINE_SIZE 128
#define ARCHIVE_PERCENT 80
#define FULL_PERCENT 100
#define ARCHIVE_LIMIT (BUFFER_SIZE * ARCHIVE_PERCENT / FULL_PERCENT)
int main (void)
{
    puts("Здравствуйте!");
	puts("Программа обрабатывает записи сервера авторизации.");
    return 0;
}