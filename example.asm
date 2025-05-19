main:
    ; Пролог функции
    push ebp
    mov ebp, esp
    sub esp, 8      ; Выделяем место для выравнивания стека
    
    ; ===== Вызов и вывод f1 =====
    ; Подготовка аргумента для f1
    fld qword [test_value1]
    sub esp, 8
    fstp qword [esp]    ; Передаем значение на стек
    
    call f1             ; Вызываем f1
    
    ; Подготовка аргументов для printf
    sub esp, 8
    fstp qword [esp]    ; Результат f1 -> второй аргумент printf
    
    fld qword [test_value1]
    sub esp, 8
    fstp qword [esp]    ; Значение test_value1 -> первый аргумент printf
    
    push format_f1      ; Строка формата
    call printf
    add esp, 24         ; Очищаем стек (8+8+8 байт)
    
    ; ===== Вызов и вывод f2 =====
    ; Подготовка аргумента для f2
    fld qword [test_value2]
    sub esp, 8
    fstp qword [esp]    ; Передаем значение на стек
    
    call f2             ; Вызываем f2
    
    ; Подготовка аргументов для printf
    sub esp, 8
    fstp qword [esp]    ; Результат f2 -> второй аргумент printf
    
    fld qword [test_value2]
    sub esp, 8
    fstp qword [esp]    ; Значение test_value2 -> первый аргумент printf
    
    push format_f2      ; Строка формата
    call printf
    add esp, 24         ; Очищаем стек (8+8+8 байт)
    
    ; ===== Вызов и вывод f3 =====
    call f3             ; Вызываем f3
    
    ; Подготовка аргументов для printf
    sub esp, 8
    fstp qword [esp]    ; Результат f3 -> аргумент printf
    
    push format_f3      ; Строка формата
    call printf
    add esp, 12         ; Очищаем стек (8+4 байт)
    
    ; Эпилог функции
    mov esp, ebp
    pop ebp
    xor eax, eax        ; Возвращаем 0
    ret