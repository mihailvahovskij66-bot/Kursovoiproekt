#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#define BUFFER_SIZE 10
#define NODE_SIZE 32
#define LINE_SIZE 128
#define FIELD_COUNT 5
#define ARCHIVE_PERCENT 80
#define FULL_PERCENT 100
#define ARCHIVE_LIMIT (BUFFER_SIZE * ARCHIVE_PERCENT / FULL_PERCENT)
typedef struct event {
	int h;				/* Час события. */
	int m;				/* Минута события. */
	char node[NODE_SIZE];		/* Название узла. */
	int logins;			/* Количество входов. */
	int failures;			/* Неудачные попытки входа. */
} Event;
int main (void)
{
    char line[LINE_SIZE];
    Event current;			/* Текущая запись. */
	int fields;			/* Количество прочитанных полей. */
	char extra;			/* Проверка лишних символов. */
    system("chcp 65001 > nul");	
    puts("Здравствуйте!");
	puts("Программа обрабатывает записи сервера авторизации.");
    puts("Введите сообщение: час:минута node logins failures");
    if (fgets(line, LINE_SIZE, stdin) == NULL) {
		puts("Не удалось прочитать сообщение.");
		return 1;
	}
    fields = sscanf(line, "%9d:%9d %31s %9d %9d %c",
		&current.h, &current.m, current.node,
		&current.logins, &current.failures, &extra);
	if (fields != FIELD_COUNT) {
		puts("Неверный формат сообщения.");
		return 1;
	}
    printf("Вы ввели: %s", line);
    printf("Час: %d\n", current.h);
	printf("Минута: %d\n", current.m);	
    printf("Узел: %s\n", current.node);
    printf("Количество входов: %d\n", current.logins);
    printf("Неудачные попытки: %d\n", current.failures);
    return 0;
}