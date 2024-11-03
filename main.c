#include <stdlib.h>
#include <stdio.h>

//структура для хранения символа, частоты и кода
typedef struct {
    char symbol;
    int frequency;
    char *code;
} Symbol;

//функция для обмена двух элементов массива
void swap(Symbol *a, Symbol *b) {
    Symbol temp = *a;
    *a = *b;
    *b = temp;
}

//функция сортировки пузырьком по частоте
void sort(Symbol arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j].frequency < arr[j + 1].frequency)
                swap(&arr[j], &arr[j + 1]);
        }
    }
}

//функция для нахождения суммы частот в массиве
int sumFrequencies(Symbol arr[], int start, int end) {
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += arr[i].frequency;
    }
    return sum;
}

//реализация алгоритма шеннона-фано
void fano(Symbol arr[], int start, int end) {
    if (start >= end) {
        return;
    }

    //нахождение точки разделения массива
    int totalSum = sumFrequencies(arr, start, end);
    int currentSum = 0;
    int splitIndex = start;

    for (int i = start; i <= end; i++) {
        currentSum += arr[i].frequency;
        if (currentSum >= totalSum / 2) {
            splitIndex = i;
            break;
        }
    }

    //присвоение кода 0 для первой части и 1 для второй
    for (int i = start; i <= splitIndex; i++) {
        int length = 0;
        while (arr[i].code[length] != '\0') length++;
        arr[i].code[length] = '0';
        arr[i].code[length + 1] = '\0';
    }

    for (int i = splitIndex + 1; i <= end; i++) {
        int length = 0;
        while (arr[i].code[length] != '\0') length++;
        arr[i].code[length] = '1';
        arr[i].code[length + 1] = '\0';
    }
    //рекурсивное применение
    fano(arr, start, splitIndex);
    fano(arr, splitIndex + 1, end);
}

//функция для поиска кода символа
char* findCode(Symbol symbols[], int count, char ch) {
    for (int i = 0; i < count; i++) {
        if (symbols[i].symbol == ch)
            return symbols[i].code;
    }
    return "";//если символ не найден
}

//функция, которая возвращает массив байтами
unsigned char* readFile(const char *fileName, size_t *size) {
    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        perror("File open error");
        return NULL;
    }

    unsigned char *byteArray = NULL;//массив байтов
    unsigned char byte = 0;//хранение байта
    int bitPos = 7;//позиция бита(слева направо)
    size_t capacity = 1;// Начальный размер массива
    size_t byteCount = 0;// Количество заполненных байтов

    //начальная память 1 байт
    byteArray = malloc(capacity * sizeof(unsigned char));
    if (byteArray == NULL) {
        perror("Memory allocation error");
        free(byteArray);
        fclose(file);
        return NULL;
    }

    int c = 0;
    while ((c = fgetc(file)) != EOF) {
        //сдвигаем бит на bitPos и добавляем в byte
        byte |= (c - '0') << bitPos;
        bitPos--;
        //если все 8 бит заполнены
        if (bitPos < 0) {
            //нужно ли увеличить размер массив????
            if (byteCount >= capacity) {
                capacity *= 2;
                unsigned char *temp = realloc(byteArray, capacity * sizeof(unsigned char));
                if (temp == NULL) {
                    perror("Memory reallocation error");
                    free(byteArray);
                    fclose(file);
                    return NULL;
                }
                byteArray = temp;
            }
            //сохранение байта и сброс
            byteArray[byteCount++] = byte;
            byte = 0;
            bitPos = 7;
        }
    }

    //проверка на неполный байт в конце файла
    if (bitPos < 7) {
        byteArray[byteCount++] = byte;
    }

    *size = byteCount;//устанавливаем количество байтов в возвращаемой переменной
    fclose(file);
    return byteArray;
}

void writeChar(unsigned char *byteArray, size_t byteArraySize, const char *outputFileName) {
    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        perror("File open error");
        return;
    }

    //запись символов в файл
    for (int i = 0; i < byteArraySize; i++) {
        fputc(byteArray[i], outputFile);//запись символа
    }

    fclose(outputFile);
}

//обратно
// Функция для декодирования массива байтов обратно в текстовую строку
void decodeToText(const unsigned char *byteArray, int byteArraySize, Symbol *symbols, int symbolCount,int len, char *outputFileName) {
    FILE *outputFile = fopen(outputFileName, "w");
    if (outputFile == NULL) {
        perror("File open error");
        return;
    }

    //буфер для хранения текущего кода символа
    char codeBuffer[32];
    int codeBufferIndex = 0;
    int sov = 0;

    //проходим по каждому байту в массиве
    for (int i = 0; i < byteArraySize; i++) {
        //преобразуем каждый бит байта в символ 0 или 1
        for (int bit = 7; bit >= 0; bit--) {
            codeBuffer[codeBufferIndex++] = (byteArray[i] & (1 << bit)) ? '1' : '0';//если бит 1 то добавляем 1, если 0 то 0
            codeBuffer[codeBufferIndex] = '\0';//конец строки

            //сравниваем накопленный код с кодами символов в массиве символов
            for (int j = 0; j < symbolCount; j++) {
                int match = 1;
                for (int k = 0; symbols[j].code[k] != '\0' && codeBuffer[k] != '\0'; k++) {
                    if (symbols[j].code[k] != codeBuffer[k]) {
                        match = 0;
                        break;
                    }
                }

                //запись в файл если есть совпадение
                if (match && symbols[j].code[codeBufferIndex] == '\0') {
                    if(sov == len)
                        break;
                    fputc(symbols[j].symbol, outputFile);
                    codeBufferIndex = 0;//сбрасываем буфер для следующего символа
                    sov++;
                    break;
                }
            }
        }
    }

    fclose(outputFile);
}

int main() {
    FILE *file;
    char *input = NULL;
    Symbol *symbols = NULL;
    int *frequencies = NULL;
    int symbolCount = 0;
    int inputSize = 256;

    file = fopen("C:/Users/163/CLionProjects/Fano/code.txt", "wb+");//для записи кода
    if (file == NULL) {
        perror("File open error");
        return 1;
    }
    //чтение исходного текста из файла
    FILE *inputFile = fopen("C:/Users/163/CLionProjects/Fano/input.txt", "r");
    if (inputFile == NULL) {
        perror("Input file open error");
        return 1;
    }

    input = (char *)malloc(inputSize * sizeof(char));//сначала выделяем 256 байт
    if (input == NULL) {
        perror("Memory allocation error for input text\n");
        free(input);
        fclose(file);
        fclose(inputFile);
        return 1;
    }

    int len = 0;
    int c;
    while ((c = fgetc(inputFile)) != EOF) {
        if (len >= inputSize) {
            inputSize *= 2;
            char *temp = realloc(input, inputSize);//расширяем память если не хватает
            if (temp == NULL) {
                perror("Memory reallocation error for input text");
                free(input);
                fclose(file);
                fclose(inputFile);
                return 1;
            }
            input = temp;
        }
        input[len++] = (char)c;
    }
    input[len] = '\0';
    fclose(inputFile);

    //инициализация массива частот.158 строка(выделить symbolCount ячеек невозможно)
    frequencies = (int *)calloc(256, sizeof(int));
    if (frequencies == NULL) {
        perror("Error of memory allocation for frequencies\n");
        free(input);
        free(frequencies);
        fclose(file);
        return 1;
    }

    //подсчёт частоты символов
    for (int i = 0; input[i] != '\0'; i++) {
        char q = input[i];
        frequencies[(unsigned char)q]++;
    }

    //подсчёт количества уникальных символов
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            symbolCount++;
        }
    }

    // Выделение памяти для массива символов и их частот (столько, сколько нужно)
    symbols = (Symbol *)malloc(symbolCount * sizeof(Symbol));
    if (symbols == NULL) {
        perror("Error of memory allocation for symbols\n");
        free(input);
        free(frequencies);
        fclose(file);
        return 1;
    }

    // Заполнение массива символов и их частот
    int index = 0;
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            symbols[index].symbol = (char)i;
            symbols[index].frequency = frequencies[i];
            symbols[index].code = (char *)malloc(1);  // Изначально выделяем память под пустую строку
            symbols[index].code[0] = '\0';  // Устанавливаем конец строки
            index++;
        }
    }

    // Сортировка символов по частоте
    sort(symbols, symbolCount);

    // gрименение алгоритма Фано
    fano(symbols, 0, symbolCount - 1);

    // Вывод частот символов и их кодов
    printf("\nFrequencies and codes:\n");
    for (int i = 0; i < symbolCount; i++) {
        printf("Symbol '%c' (frequency %d) code: %s\n", symbols[i].symbol, symbols[i].frequency, symbols[i].code);
    }

    // Кодированный текст и запись его в файл
    printf("\nCoded text: ");
    for (int i = 0; input[i] != '\0'; i++) {
        char* code = findCode(symbols, symbolCount, input[i]);
        printf("%s", code);
        fputs(code, file);
    }
    printf("\n");
    //закрытие файла
    fclose(file);

    //byteArray - массив с байтами из файла
    int byteArraySize = 0;
    unsigned char *byteArray = readFile("C:/Users/163/CLionProjects/Fano/code.txt", &byteArraySize);
    if(byteArray == NULL){
        perror("Memory allocation error");
        free(byteArray);
    }
    printf("Byte array:\n");
    for (size_t i = 0; i < byteArraySize; i++) {
        printf("%c ", byteArray[i]);
    }
    printf("\n Size: %d", len-1);
    //запись в файл code.txt символов, коды которых находятся в byteArray
    writeChar(byteArray, byteArraySize, "C:/Users/163/CLionProjects/Fano/code.txt");

    int sizeLen = sizeof(input) - 1;//длина введенной строки
    decodeToText(byteArray,byteArraySize,symbols,symbolCount,len, "C:/Users/163/CLionProjects/Fano/decode.txt");

    //освобождение памяти
    free(input);
    free(frequencies);
    for (int i = 0; i < symbolCount; i++) {
        free(symbols[i].code);  //освобождаем память для кода каждого символа
    }
    free(symbols);

    return 0;
}