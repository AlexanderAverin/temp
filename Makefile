CFLAGS ?= -O2 -g
CFLAGS += -std=gnu99
CFLAGS += -Wall -Werror -Wformat-security -Wignored-qualifiers -Winit-self \
	-Wswitch-default -Wpointer-arith -Wtype-limits -Wempty-body \
	-Wstrict-prototypes -Wold-style-declaration -Wold-style-definition \
	-Wmissing-parameter-type -Wmissing-field-initializers -Wnested-externs \
	-Wstack-usage=4096 -Wmissing-prototypes -Wfloat-equal -Wabsolute-value
CFLAGS += -fsanitize=undefined -fsanitize-undefined-trap-on-error
CC += -m32 -no-pie -fno-pie
LDLIBS = -lm

# src
SRC_DIR = src
ASM_DIR = $(SRC_DIR)/asm
CLI_DIR = $(SRC_DIR)/cli
PARSER_DIR = $(SRC_DIR)/parser

# Объектные файлы
OBJS = integral.o $(SRC_DIR)/solver.o $(SRC_DIR)/intagrate.o \
	$(CLI_DIR)/cmdline.o $(ASM_DIR)/functions.o

# Усложненный вариант
GEN_ASM = generator
GEN_ASM_OBJS = $(GEN_ASM).o $(PARSER_DIR)/ast.o

# Спецификация функций
SPEC_FILE ?= functions.txt
GENERATED_ASM = $(ASM_DIR)/generated_functions.asm

.PHONY: all clean test run

all: integral

# Lexer
GEN_ASM_OBJS = $(GEN_ASM).o $(PARSER_DIR)/ast.o lexer.o
lexer.o: lexer.c lexer.h
	$(CC) $(CFLAGS) -c -o lexer.o lexer.c
###

# Сборка основной программы
integral: $(OBJS)
	$(CC) $(CFLAGS) -o integral $(OBJS) $(LDLIBS)

# Компиляция C файлов
integral.o: integral.c
	$(CC) $(CFLAGS) -c -o integral.o integral.c

$(SRC_DIR)/solver.o: $(SRC_DIR)/solver.c
	$(CC) $(CFLAGS) -c -o $(SRC_DIR)/solver.o $(SRC_DIR)/solver.c

$(SRC_DIR)/intagrate.o: $(SRC_DIR)/intagrate.c
	$(CC) $(CFLAGS) -c -o $(SRC_DIR)/intagrate.o $(SRC_DIR)/intagrate.c

$(CLI_DIR)/cmdline.o: $(CLI_DIR)/cmdline.c
	$(CC) $(CFLAGS) -c -o $(CLI_DIR)/cmdline.o $(CLI_DIR)/cmdline.c

$(PARSER_DIR)/ast.o: $(PARSER_DIR)/ast.c
	$(CC) $(CFLAGS) -c -o $(PARSER_DIR)/ast.o $(PARSER_DIR)/ast.c

$(GEN_ASM).o: $(GEN_ASM).c
	$(CC) $(CFLAGS) -c -o $(GEN_ASM).o $(GEN_ASM).c

# Компиляция ассемблерных файлов
$(ASM_DIR)/functions.o: $(ASM_DIR)/functions.asm
	nasm -f elf32 -o $(ASM_DIR)/functions.o $(ASM_DIR)/functions.asm

$(ASM_DIR)/generated_functions.o: $(GENERATED_ASM)
	nasm -f elf32 -o $(ASM_DIR)/generated_functions.o $(GENERATED_ASM)

# Сборка вспомогательной программы для генерации ассемблера
$(GEN_ASM): $(GEN_ASM_OBJS)
	$(CC) $(CFLAGS) -o $(GEN_ASM) $(GEN_ASM_OBJS) $(LDLIBS)

# Генерация ассемблерного файла из спецификации
$(GENERATED_ASM): $(GEN_ASM) $(SPEC_FILE)
	./$(GEN_ASM) $(SPEC_FILE) $(GENERATED_ASM)

# Вариант для использования сгенерированных функций
integral_generated: integral.c $(SRC_DIR)/solver.o $(SRC_DIR)/intagrate.o \
	$(CLI_DIR)/cmdline.o $(ASM_DIR)/generated_functions.o
	$(CC) $(CFLAGS) -o integral_generated integral.c $(SRC_DIR)/solver.o $(SRC_DIR)/intagrate.o \
	$(CLI_DIR)/cmdline.o $(ASM_DIR)/generated_functions.o $(LDLIBS)

# Тесты для root и integral
test: integral
	@echo "Testing root function:"
	./integral --test-root 1:2:0.0:2.0:0.0001:1.0
	./integral --test-root 1:3:0.0:2.0:0.0001:0.5
	./integral --test-root 2:3:0.0:2.0:0.0001:1.5
	@echo "Testing integral function:"
	./integral --test-integral 1:0.0:1.0:0.0001:2.5
	./integral --test-integral 2:0.0:1.0:0.0001:0.16667
	./integral --test-integral 3:0.0:1.0:0.0001:0.33333

# Запуск программы
run: integral
	./integral

# Выбор метода решения уравнений на этапе препроцессирования
method_bisection: CFLAGS += -DUSE_BISECTION
method_bisection: integral

method_chord: CFLAGS += -DUSE_CHORD
method_chord: integral

method_newton: CFLAGS += -DUSE_NEWTON
method_newton: integral

method_combined: CFLAGS += -DUSE_COMBINED
method_combined: integral

# Очистка
clean:
	rm -f integral integral_generated $(GEN_ASM) *.o $(SRC_DIR)/*.o $(ASM_DIR)/*.o \
	$(CLI_DIR)/*.o $(PARSER_DIR)/*.o $(GENERATED_ASM)

# AST BUILD
# SPEC_FILE=your_functions.txt make integral_generated