/* Автор: Ваховский Михаил. */

/*
 * Курсовой проект. Вариант 15: сервер авторизации.
 * Обрабатывает сообщения, отбрасывает дубликаты в буфере,
 * сохраняет записи в архив и выводит статистику.
 */

#define _CRT_SECURE_NO_WARNINGS
#define ARCHIVE_FILE "archive.txt"
#define MAX_HOUR 23
#define MAX_MINUTE 59
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
/*
 * Проверяет наличие дубликата в текущем буфере.
 * ptrbuffer - массив сохранённых записей.
 * count - количество записей в буфере.
 * current - новая запись для проверки.
 * Последовательно сравнивает поля node и failures.
 * Возвращает 1, если найдена запись с совпадающими полями.
 * Возвращает 0, если дубликат отсутствует.
 */
int is_duplicate(const Event *ptrbuffer, int count, Event current);
/*
 * Дописывает записи буфера в текстовый файл архива.
 * ptrbuffer - массив записей для сохранения.
 * count - количество сохраняемых записей.
 * Открывает файл ARCHIVE_FILE в режиме дописывания.
 * Записывает каждое событие отдельной строкой и закрывает файл.
 * Создаёт файл, если он отсутствует.
 * Возвращает count при успешном сохранении.
 * Возвращает -1 при ошибке открытия, записи или закрытия файла.
 */
int archive_events(const Event *ptrbuffer, int count);
int main (void)
{
	char line[LINE_SIZE];		/* Строка сообщения. */
	Event current;			/* Текущая запись. */
	int fields;			/* Количество прочитанных полей. */
	char extra;			/* Проверка лишних символов. */
	int next_char;			/* Остаток слишком длинной строки. */

	Event buffer[BUFFER_SIZE];	/* Буфер записей сервера. */
	int buffer_count = 0;
	int received_count = 0;
	int duplicate_count = 0;		/* Количество отброшенных дубликатов. */		/* Количество полученных корректных записей. */		/* Количество записей в буфере. */
	int i;
	int written_count;		/* Результат записи буфера в архив. */
	int archived_count = 0;		/* Записано в архив за текущий запуск. */

	system("chcp 65001 > nul");
	puts("Здравствуйте!");
	puts("Программа обрабатывает записи сервера авторизации.");
	puts("Для завершения нажмите Enter на пустой строке.");

	while (1) {
		puts("Введите сообщение: час:минута node logins failures");
		if (fgets(line, LINE_SIZE, stdin) == NULL) {
			break;
		}
		if (line[0] == '\n') {
			break;
		}
		if (strchr(line, '\n') == NULL && !feof(stdin)) {
			while ((next_char = getchar()) != '\n' && next_char != EOF) {
				/* Пропускаем остаток слишком длинного сообщения. */
			}
			puts("Сообщение слишком длинное.");
			continue;
		}

		fields = sscanf(line, "%9d:%9d %31s %9d %9d %c",
			&current.h, &current.m, current.node,
			&current.logins, &current.failures, &extra);
		if (fields != FIELD_COUNT) {
			puts("Неверный формат сообщения.");
			continue;
		}
		if (current.h < 0 || current.h > MAX_HOUR) {
			puts("Час должен быть от 0 до 23.");
			continue;
		}
		if (current.m < 0 || current.m > MAX_MINUTE) {
			puts("Минута должна быть от 0 до 59.");
			continue;
		}
		if (current.logins < 0) {
			puts("Количество входов должно быть не меньше нуля.");
			continue;
		}
		if (current.failures < 0) {
			puts("Количество неудачных попыток должно быть не меньше нуля.");
			continue;
		}
		received_count++;
		if (is_duplicate(buffer, buffer_count, current) == 1) {
			duplicate_count++;
			puts("Запись является дубликатом и не сохранена.");
			continue;
		}

		if (buffer_count >= BUFFER_SIZE) {
			puts("Буфер заполнен. Запись не сохранена.");
			continue;
		}
		buffer[buffer_count] = current;
		buffer_count++;
		printf("Записей в буфере: %d из %d\n", buffer_count, BUFFER_SIZE);

		printf("Вы ввели: %s", line);
		printf("Час: %d\n", current.h);
		printf("Минута: %d\n", current.m);
		printf("Узел: %s\n", current.node);
		printf("Количество входов: %d\n", current.logins);
		printf("Неудачные попытки: %d\n", current.failures);
		if (buffer_count >= ARCHIVE_LIMIT) {
			written_count = archive_events(buffer, buffer_count);
			if (written_count < 0) {
				puts("Не удалось записать данные в архив.");
				return 1;
			}
			archived_count += written_count;
			buffer_count = 0;
			printf("В архив записано записей: %d\n", written_count);
		}		
	}

	if (ferror(stdin)) {
		puts("Не удалось прочитать сообщение.");
		return 1;
	}
	if (buffer_count > 0) {
		written_count = archive_events(buffer, buffer_count);
		if (written_count < 0) {
			puts("Не удалось сохранить оставшиеся записи в архив.");
			return 1;
		}
		archived_count += written_count;
		buffer_count = 0;
		puts("Оставшиеся записи сохранены в архив.");
	}

	printf("Получено записей: %d\n", received_count);
	printf("Отброшено дубликатов: %d\n", duplicate_count);
	printf("Записано в архив: %d\n", archived_count);
	puts("Записи, оставшиеся в буфере:");
	for (i = 0; i < buffer_count; i++) {
		printf("%02d:%02d %s %d %d\n",
			buffer[i].h, buffer[i].m, buffer[i].node,
			buffer[i].logins, buffer[i].failures);
	}

	return 0;
}
int is_duplicate(const Event *ptrbuffer, int count, Event current) {
	int i;

	for (i = 0; i < count; i++) {
		if (strcmp(ptrbuffer[i].node, current.node) == 0 &&
			ptrbuffer[i].failures == current.failures) {
			return 1;
		}
	}

	return 0;
}
int archive_events(const Event *ptrbuffer, int count) {
	FILE *ptrarchive;		/* Файл архива. */
	int i;

	ptrarchive = fopen(ARCHIVE_FILE, "a");
	if (ptrarchive == NULL) {
		return -1;
	}

	for (i = 0; i < count; i++) {
		if (fprintf(ptrarchive, "%02d:%02d %s %d %d\n",
			ptrbuffer[i].h, ptrbuffer[i].m, ptrbuffer[i].node,
			ptrbuffer[i].logins, ptrbuffer[i].failures) < 0) {
			fclose(ptrarchive);
			return -1;
		}
	}

	if (fclose(ptrarchive) == EOF) {
		return -1;
	}

	return count;
}
