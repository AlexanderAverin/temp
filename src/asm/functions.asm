; f1(x) = 2^x + 1
; f2(x) = x^5
; f3(x) = (1-x)/3

section .rodata
    ; consts data

    const_1 dq 1.0
    const_3 dq 3.0
    const_5 dq 5.0

    const_ln2 dq 0.693147180559945  ; ln(2)
    const_minus_1_div_3 dq -0.333333333333333  ; -1/3

section .text
    global f1
    global f2
    global f3
    global df1
    global df2
    global df3

; --------------------------------------------------------------
; f1(x) = 2^x + 1
; --------------------------------------------------------------

f1:
    push ebp
    mov ebp, esp
    
    ; load x
    fld qword[ebp + 8]
    
    ; calc 2^x
    ; use f2xm1 for calc 2^x - 1
    ; for this separated x on int and float parts


    fld st0            ; "copy * 2" x => st0=x, st1=x
    fld st0            ; again "copy * 2" => st0=x, st1=x, st2=x
    frndint            ; round to int => st0=int(x), st1=x, st2=x
    fxch st1           ; swap =>  st0=x, st1=int(x), st2=x
    fsub st0, st1      ; minus float part, int part not mutant => st0=frac(x), st1=int(x), st2=x
    f2xm1              ; calc 2^(float part) - 1: => st0=2^frac(x)-1, st1=int(x), st2=x
    fld1               ; load 1.0 => st0=1.0, st1=2^frac(x)-1, st2=int(x), st3=x
    faddp              ; add with 1 => have  2^(float part) => st0=2^frac(x), st1=int(x), st2=x
    fscale             ; mult on 2^(int part) => st0=2^x, st1=int(x), st2=x
    fstp st1           ; remove st1 => now st0=2^x, st1=x
    fstp st1           ; again remove st1 => now st0=2^x
    
    ; add 1
    fld qword[const_1]
    faddp
    
    pop ebp
    ret

; --------------------------------------------------------------
; df1(x) = 2^x * ln(2)
; --------------------------------------------------------------

df1:
    push ebp
    mov ebp, esp
    
    ; Загружаем x
    fld qword [ebp + 8]
    
    ; Вычисляем 2^x (аналогично f1, но без добавления 1)
    fld st0            ; Дублируем x: st0=x, st1=x
    fld st0            ; Еще раз дублируем: st0=x, st1=x, st2=x
    frndint            ; Округляем до целого: st0=int(x), st1=x, st2=x
    fxch st1           ; Меняем местами: st0=x, st1=int(x), st2=x
    fsub st0, st1      ; Вычитаем целую часть: st0=frac(x), st1=int(x), st2=x
    f2xm1              ; Вычисляем 2^(дробная часть) - 1: st0=2^frac(x)-1, st1=int(x), st2=x
    fld1               ; Загружаем 1.0: st0=1.0, st1=2^frac(x)-1, st2=int(x), st3=x
    faddp              ; Складываем с 1: st0=2^frac(x), st1=int(x), st2=x
    fscale             ; Умножаем на 2^(целая часть): st0=2^x, st1=int(x), st2=x
    fstp st1           ; Убираем st1, теперь st0=2^x, st1=x
    fstp st1           ; Убираем st1, теперь st0=2^x
    
    ; Умножаем на ln(2)
    fld qword [const_ln2]
    fmulp
    
    pop ebp
    ret

; --------------------------------------------------------------
; f2(x) = x^5
; --------------------------------------------------------------
f2:
    push ebp
    mov ebp, esp
    
    ; Загружаем x
    fld qword[ebp + 8]
    
    ; Вычисляем x^2
    fld st0
    fmulp       ; st0 = x^2
    
    ; Вычисляем x^4
    fld st0
    fmulp       ; st0 = x^4
    
    ; Умножаем на x, получаем x^5
    fld qword[ebp + 8]
    fmulp       ; st0 = x^5
    
    pop ebp
    ret

; --------------------------------------------------------------
; df2(x) = 5*x^4
; --------------------------------------------------------------
df2:
    push ebp
    mov ebp, esp
    
    ; Загружаем x
    fld qword [ebp + 8]
    
    ; Вычисляем x^2
    fld st0
    fmulp       ; st0 = x^2
    
    ; Вычисляем x^4
    fld st0
    fmulp       ; st0 = x^4
    
    ; Умножаем на 5
    fld qword [const_5]
    fmulp       ; st0 = 5*x^4
    
    pop ebp
    ret

; --------------------------------------------------------------
; f3(x) = (1-x)/3
; --------------------------------------------------------------
f3:
    push ebp
    mov ebp, esp
    
    ; Загружаем x
    fld qword [ebp + 8]
    
    ; Вычисляем 1-x
    fld qword [const_1]
    fsubr      ; st0 = 1-x (обратное вычитание)
    
    ; Делим на 3
    fld qword [const_3]
    fdivp      ; st0 = (1-x)/3
    
    pop ebp
    ret

; --------------------------------------------------------------
; df3(x) = -1/3
; --------------------------------------------------------------
df3:
    push ebp
    mov ebp, esp
    
    ; Просто загружаем константу -1/3
    fld qword [const_minus_1_div_3]
    
    pop ebp
    ret