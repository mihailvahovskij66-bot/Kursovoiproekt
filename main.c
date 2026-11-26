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
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>

#define BUFFER_SIZE 10
#define NODE_SIZE 32
#define LINE_SIZE 128
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
int archive_events(const Event *ptrbuffer, int count);
static int parse_event(const char *ptrline, Event *ptrevent);
static int parse_int_field(const char *ptrstart, const char *ptrend,
	int *ptrvalue);

int main (void)
{
	char line[LINE_SIZE];		/* Строка сообщения. */
	Event current;			/* Текущая запись. */
	int next_char;			/* Остаток слишком длинной строки. */

	Event buffer[BUFFER_SIZE];	/* Буфер записей сервера. */
	int buffer_count = 0;
	int received_count = 0;
	int duplicate_count = 0;		/* Количество отброшенных дубликатов. */		/* Количество полученных корректных записей. */		/* Количество записей в буфере. */
	int i;
	int written_count;		/* Результат записи буфера в архив. */
	int archived_count = 0;		/* Записано в архив за текущий запуск. */
	int invalid_count = 0;		/* Количество некорректных сообщений. */
	
	system("chcp 65001 > nul");
	puts("Здравствуйте!");
	puts("Программа обрабатывает записи сервера авторизации.");
	puts("Для завершения нажмите Enter на пустой строке.");
	puts("Для подсказки введите /help.");
	puts("Для просмотра статистики введите /stats.");
	puts("Для просмотра записей в буфере введите /buffer.");
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
			invalid_count++;
			puts("Сообщение слишком длинное.");
			continue;
		}
		if (strcmp(line, "/help\n") == 0 ||
			strcmp(line, "/help") == 0) {
			puts("Формат: час:минута node logins failures");
			puts("Пример: 09:30 node1 12 2");
			printf("Час: от 0 до %d. Минута: от 0 до %d.\n",
				MAX_HOUR, MAX_MINUTE);
			puts("node — название узла без пробелов.");
			puts("logins и failures — целые числа от нуля.");
			puts("Команда /stats показывает текущую статистику.");
			puts("Команда /buffer показывает записи текущего буфера.");			
			puts("Для завершения нажмите Enter на пустой строке.");
			continue;
		}
		if (strcmp(line, "/stats\n") == 0 ||
			strcmp(line, "/stats") == 0) {
			puts("Текущая статистика:");
			printf("Получено записей: %d\n", received_count);
			printf("Отброшено дубликатов: %d\n", duplicate_count);
			printf("Некорректных сообщений: %d\n", invalid_count);
			printf("Записано в архив: %d\n", archived_count);
			printf("Записей в буфере: %d из %d\n", buffer_count, BUFFER_SIZE);
			continue;
		}
		if (strcmp(line, "/buffer\n") == 0 ||
			strcmp(line, "/buffer") == 0) {
			puts("Текущий буфер:");
			if (buffer_count == 0) {
				puts("Буфер пуст.");
			}
			for (i = 0; i < buffer_count; i++) {
				printf("%02d:%02d %s %d %d\n",
					buffer[i].h, buffer[i].m, buffer[i].node,
					buffer[i].logins, buffer[i].failures);
			}
			continue;
		}

				if (!parse_event(line, &current)) {
			invalid_count++;
			puts("Неверный формат сообщения.");
			continue;
		}
		if (current.m < 0 || current.m > MAX_MINUTE) {
			invalid_count++;
			puts("Минута должна быть от 0 до 59.");
			continue;
		}
		if (current.logins < 0) {
			invalid_count++;
			puts("Количество входов должно быть не меньше нуля.");
			continue;
		}
		if (current.failures < 0) {
			invalid_count++;
			puts("Количество неудачных попыток должно быть не меньше нуля.");
			continue;
		}
		received_count++;

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
		duplicate_count += buffer_count - written_count;
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
		duplicate_count += buffer_count - written_count;
		archived_count += written_count;
		buffer_count = 0;
		puts("Оставшиеся записи сохранены в архив.");
	}

	printf("Получено записей: %d\n", received_count);
	printf("Отброшено дубликатов: %d\n", duplicate_count);
	printf("Записано в архив: %d\n", archived_count);
	puts("Записи, оставшиеся в буфере:");
	printf("Некорректных сообщений: %d\n", invalid_count);
	for (i = 0; i < buffer_count; i++) {
		printf("%02d:%02d %s %d %d\n",
			buffer[i].h, buffer[i].m, buffer[i].node,
			buffer[i].logins, buffer[i].failures);
	}

	return 0;
}
/*
 * Разбирает строку ptrline и заполняет запись ptrevent.
 * Проверяет число полей, разделители и длину имени узла.
 * Возвращает 1 при успехе или 0 при ошибке разбора.
 */
static int parse_event(const char *ptrline, Event *ptrevent) {
	int time_end;		/* Конец поля времени. */
	int node_start;		/* Начало имени узла. */
	int node_end;		/* Конец имени узла. */
	int logins_start;	/* Начало поля logins. */
	int logins_end;		/* Конец поля logins. */
	int failures_start;	/* Начало поля failures. */
	int failures_end;	/* Конец поля failures. */
	int consumed = 0;	/* Длина разобранной части строки. */
	const char *ptrcolon;	/* Двоеточие между часом и минутой. */
	size_t node_length;	/* Длина имени узла. */

	/* %*s пропускает поле целиком, а %n сохраняет позицию в строке. */
	(void)sscanf(ptrline, "%*s%n %n%*s%n %n%*s%n %n%*s%n %n",
		&time_end, &node_start, &node_end, &logins_start, &logins_end,
		&failures_start, &failures_end, &consumed);

	if (consumed == 0 || ptrline[consumed] != '\0') {
		return 0;
	}

	ptrcolon = strchr(ptrline, ':');
	node_length = (size_t)(node_end - node_start);
	if (ptrcolon == NULL || ptrcolon >= ptrline + time_end ||
		node_length >= sizeof(ptrevent->node)) {
		return 0;
	}

	if (!parse_int_field(ptrline, ptrcolon, &ptrevent->h) ||
		!parse_int_field(ptrcolon + 1, ptrline + time_end, &ptrevent->m) ||
		!parse_int_field(ptrline + logins_start, ptrline + logins_end,
			&ptrevent->logins) ||
		!parse_int_field(ptrline + failures_start, ptrline + failures_end,
			&ptrevent->failures)) {
		return 0;
	}

	memcpy(ptrevent->node, ptrline + node_start, node_length);
	ptrevent->node[node_length] = '\0';
	return 1;
}

/*
 * Разбирает целое число от ptrstart до позиции ptrend.
 * При успехе сохраняет значение через ptrvalue и возвращает 1.
 * Возвращает 0 при неверном формате или выходе за диапазон int.
 */
static int parse_int_field(const char *ptrstart, const char *ptrend,
	int *ptrvalue) {
	char *ptrtail;	/* Первый символ после разобранного числа. */
	long value;	/* Число до проверки диапазона int. */

	errno = 0;
	value = strtol(ptrstart, &ptrtail, 10);
	if (ptrtail == ptrstart || ptrtail != ptrend || errno == ERANGE ||
		value < INT_MIN || value > INT_MAX) {
		return 0;
	}

	*ptrvalue = (int)value;
	return 1;
}

/*
 * Проверяет, есть ли дубликат в текущем буфере.
 * ptrbuffer - массив записей, count - количество записей в нём.
 * current - запись, которую нужно проверить.
 * Возвращает 1 при совпадении node и failures, иначе 0.
 */
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
/*
 * Отбирает уникальные события и дописывает их в файл ARCHIVE_FILE.
 * ptrbuffer - массив событий, count - количество событий в массиве.
 * Для каждой пары node и failures сохраняет первое событие.
 * Возвращает число записанных событий или -1 при ошибке работы с файлом.
 */
int archive_events(const Event *ptrbuffer, int count) {
	Event unique_events[BUFFER_SIZE];	/* Записи для архива. */
	int unique_count = 0;			/* Число уникальных записей. */
	FILE *ptrarchive;			/* Файл архива. */
	int i;

	for (i = 0; i < count; i++) {
		if (is_duplicate(unique_events, unique_count, ptrbuffer[i]) == 0) {
			unique_events[unique_count] = ptrbuffer[i];
			unique_count++;
		}
	}

	ptrarchive = fopen(ARCHIVE_FILE, "a");
	if (ptrarchive == NULL) {
		return -1;
	}

	for (i = 0; i < unique_count; i++) {
		if (fprintf(ptrarchive, "%02d:%02d %s %d %d\n",
			unique_events[i].h, unique_events[i].m, unique_events[i].node,
			unique_events[i].logins, unique_events[i].failures) < 0) {
			fclose(ptrarchive);
			return -1;
		}
	}

	if (fclose(ptrarchive) == EOF) {
		return -1;
	}

	return unique_count;
}