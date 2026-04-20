#ifndef __ff_h__
#define __ff_h__

#ifndef FF_MEMORYCELL_MAX
    #define FF_MEMORYCELL_MAX 16
#endif

#ifndef FF_CALLBACK_MAX
    #define FF_CALLBACK_MAX 16
#endif

typedef struct ffState {
    void** stack;
    long cap, top;

    void* memory[FF_MEMORYCELL_MAX];
    long mem_cap;

    char string_cell[256];

    struct {
        char name[FF_CALLBACK_MAX][16];
        void (*ptr[FF_CALLBACK_MAX])(void);
    } fn;
    long fn_cap, fn_top;
    //int fail_flag; henüz kullanmadım ama bakılır yani
} ffState;

ffState* ff_init(int stack_size);
void ff_deinit(ffState* s);

#define pref_type long long
#define upref_type unsigned long

#define ff_pushf(vm, val) ff_push(vm, (void*)(pref_type)*(upref_type*)&(float){val})
#define ff_popf(vm) (*(float*)&(upref_type){(upref_type)(pref_type)ff_pop(vm)})

void ff_push(ffState* s, void* val);
void* ff_pop(ffState* s);
void ff_store(ffState* s, int index, void* val);
void* ff_load(ffState* s, int index);
void ff_replaceString(ffState* s, const char* str);
const char* ff_getString(ffState* s);

void ff_doString(ffState* s, const char* str);
void ff_doFile(ffState* s, const char* path);

void ff_registerFunction(ffState* s, const char* name, void(*f)(void));

void ff_clearStack(ffState* s);
void ff_clearMemory(ffState* s);

#endif
#ifdef FFVM_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef FFVM_DEBUG
    #define PRINT(str, ...) printf(str, ##__VA_ARGS__)
#else
    #define PRINT(str, ...) 
#endif

// TODO: floor, ceil, round

void __ff_eval(ffState* s, const char* token) {

    /* ================= STACK & MEMORY ================= */
    if (strncmp(token, "dup", 3) == 0) {
        void* a = ff_pop(s);
        ff_push(s, a);
        ff_push(s, a);
    }
    else if (strncmp(token, "swp", 3) == 0) {
        void* b = ff_pop(s);
        void* a = ff_pop(s);
        ff_push(s, b);
        ff_push(s, a);
    }
    else if (strncmp(token, "ldt", 3) == 0) {
        pref_type a = (pref_type)ff_pop(s);
        if (a >= s->mem_cap || a < 0) {
            PRINT("INFO: invalid memory location\n");
            ff_push(s, (void*)a);
            return;
        }
        ff_push(s, (void*)s->memory[a]);
    }
    else if (strncmp(token, "stt", 3) == 0) {
        pref_type b = (pref_type)ff_pop(s);
        void* a = ff_pop(s);
        if (b >= s->mem_cap || b < 0) {
            PRINT("INFO: invalid memory location\n");
            ff_push(s, a);
            ff_push(s, (void*)b);
            return;
        }
        s->memory[b] = a;
    }

    /* ================= LOGIC & ARITMETIC ================= */
    else if (strncmp(token, "shl", 3) == 0 || strncmp(token, "sal", 3) == 0) {
        pref_type b = (pref_type)ff_pop(s);
        pref_type a = (pref_type)ff_pop(s);
        b = b & 63;
        ff_push(s, (void*)(a << b));
    }
    else if (strncmp(token, "sar", 3) == 0) {
        pref_type b = (pref_type)ff_pop(s);
        pref_type a = (pref_type)ff_pop(s);
        ff_push(s, (void*)(a >> b));
    }
    else if (strncmp(token, "shr", 3) == 0) {
        pref_type b = (pref_type)ff_pop(s);
        upref_type a = (upref_type)ff_pop(s);
        ff_push(s, (void*)(a >> b));
    }
    else if (strncmp(token, "and", 3) == 0) {
        pref_type b = (pref_type)ff_pop(s);
        pref_type a = (pref_type)ff_pop(s);
        ff_push(s, (void*)(a & b));
    }
    else if (strncmp(token, "or", 2) == 0) {
        pref_type b = (pref_type)ff_pop(s);
        pref_type a = (pref_type)ff_pop(s);
        ff_push(s, (void*)(a | b));
    }
    else if (strncmp(token, "xor", 3) == 0) {
        pref_type b = (pref_type)ff_pop(s);
        pref_type a = (pref_type)ff_pop(s);
        ff_push(s, (void*)(a ^ b));
    }
    else if (strncmp(token, "not", 3) == 0) {
        pref_type a = (pref_type)ff_pop(s);
        ff_push(s, (void*)(~a));
    }
    else if (strncmp(token, "neg", 3) == 0) {
        pref_type a = (pref_type)ff_pop(s);
        ff_push(s, (void*)(-a));
    }

    /* ================= INTEGER ================= */
    else if (strncmp(token, "add", 3) == 0) {
        pref_type a = (pref_type)ff_pop(s);
        pref_type b = (pref_type)ff_pop(s);
        ff_push(s, (void*)(a + b));
    }
    else if (strncmp(token, "sub", 3) == 0) {
        pref_type b = (pref_type)ff_pop(s);
        pref_type a = (pref_type)ff_pop(s);
        ff_push(s, (void*)(a - b));
    }
    else if (strncmp(token, "mul", 3) == 0) {
        pref_type a = (pref_type)ff_pop(s);
        pref_type b = (pref_type)ff_pop(s);
        ff_push(s, (void*)(a * b));
    }
    else if (strncmp(token, "div", 3) == 0) {
        pref_type b = (pref_type)ff_pop(s);
        pref_type a = (pref_type)ff_pop(s);
        if (b == 0) {
            PRINT("INFO: divide by zero\n");
            return;
        }
        ff_push(s, (void*)(a / b));
    }
    else if (strncmp(token, "mad", 3) == 0) {
        pref_type c = (pref_type)ff_pop(s);
        pref_type b = (pref_type)ff_pop(s);
        pref_type a = (pref_type)ff_pop(s);
        ff_push(s, (void*)(a * b + c));
    }

    /* ================= FLOAT ================= */
    else if (strncmp(token, "fadd", 4) == 0) {
        float b = ff_popf(s);
        float a = ff_popf(s);
        ff_pushf(s, a + b);
    }
    else if (strncmp(token, "fsub", 4) == 0) {
        float b = ff_popf(s);
        float a = ff_popf(s);
        ff_pushf(s, a - b);
    }
    else if (strncmp(token, "fmul", 4) == 0) {
        float b = ff_popf(s);
        float a = ff_popf(s);
        ff_pushf(s, a * b);
    }
    else if (strncmp(token, "fdiv", 4) == 0) {
        float b = ff_popf(s);
        float a = ff_popf(s);
        if (b == 0) {
            PRINT("INFO: divide by zero\n");
            return;
        }
        ff_pushf(s, a / b);
    }
    else if (strncmp(token, "fmad", 4) == 0) {
        float c = ff_popf(s);
        float b = ff_popf(s);
        float a = ff_popf(s);
        ff_pushf(s, (a * b + c));
    }
    else if (strncmp(token, "rcp", 3) == 0) {
        float a = ff_popf(s);
        ff_pushf(s, 1 / a);
    }
    else if (strncmp(token, "rsq", 3) == 0) {
        unsigned int i = (unsigned int)(pref_type)ff_pop(s);
        float y = *(float*)&i;
        float x2 = y * 0.5f;
        i = 0x5f375a86 - (i >> 1);
        y = *(float*)&i;
        y = y * (1.5f - (x2 * y * y));
        unsigned int a = *(unsigned int*)&y;
        ff_push(s, (void*)(pref_type)a);
    }


    else {
        PRINT("INFO: Unknown Token (%s)\n", token);
    }
}

ffState* ff_init(int stack_size) {
    ffState* s = calloc(1, sizeof(ffState));
    s->cap = stack_size;
    s->stack = calloc(s->cap, sizeof(void*));
    s->top = -1;
    s->mem_cap = FF_MEMORYCELL_MAX;
    s->fn_cap = FF_CALLBACK_MAX;
    PRINT("INFO:\n\tSTACK CAPACITY:%d\n\tMEMORY CAPACITY:%d\n\tFUNCTION CAPACITY:%d\n", (int)s->cap, (int)s->mem_cap, (int)s->fn_cap);
    return s;
}

void ff_deinit(ffState *s) {
    if (s->stack) free(s->stack);
    if (s) free(s);
}

void ff_push(ffState* s, void* val) {
    if (s->stack == NULL) {
        PRINT("INFO: (STACK) NULL POINTER\n", (int)val);
        return;
    };
    if (s->top >= s->cap - 1) {
        PRINT("INFO: VALUE (%d) OVERFLOWED!\n", (int)val);
        return;
    };
    s->stack[++s->top] = val;
}

void* ff_pop(ffState* s) {
    if (s->top < 0) {
        PRINT("INFO: STACK UNDERFLOWED!\n");
        return (void*)0;
    };
    return s->stack[s->top--];
}

void ff_replaceString(ffState *s, const char *str) {
    for (int i = 0; i < 256; i++) {
        if (str[i] == '_') {
            s->string_cell[i] = ' ';
        }
        else if (str[i] == '\0') {
            s->string_cell[i] = '\0';
            break;
        }
        else {
            s->string_cell[i] = str[i];
        }
    }
    s->string_cell[255] = '\0';
}

const char* ff_getString(ffState *s) {
    return s->string_cell;
}

int ff_isAlpha(char c) {
    return (c >= 'A' && c <= 'z');
}

int ff_isSpace(char c) {
    return (c == ' ');
}

int ff_isNonsense(char c) {
    return (c == ' ' || c == '\t' || c == '\n');
}

int ff_isNumber(char c) {
    return (c >= '0' && c <= '9');
}

int ff_isSign(char c) {
    return (c == '+' || c == '-');
}

void ff_doString(ffState *s, const char *str) {
    long len = strlen(str);
    for (int i = 0; i < len; i++) {
        for (;ff_isNonsense(str[i]) && i < len;) i++;

        if (str[i] == '(') {
            for (;str[i] != ')' && i < len;) i++;
        }
        else if (str[i] == '\'') {
            i++;
            char token[256] = {0};
            for (int j = 0;!ff_isNonsense(str[i]);) {
                token[j++] = str[i++];
            }
            ff_replaceString(s, token);
        }
        else if (str[i] == '@') {
            i++;
            char token[256] = {0};
            for (int j = 0;!ff_isNonsense(str[i]);) {
                token[j++] = str[i++];
            }
            for (int j = 0; j < s->fn_cap; j++) {
                if (strcmp(token, s->fn.name[j]) == 0) {
                    s->fn.ptr[j]();
                }
            }
            
        }
        else if (ff_isAlpha(str[i])) {
            char token[64] = {0};
            for (int j = 0;ff_isAlpha(str[i]);) {
                token[j++] = str[i++];
            }
            __ff_eval(s, token);
        }
        else if (ff_isNumber(str[i]) || ff_isSign(str[i])) {
            char token[64] = {0};
            for (int j = 0;!ff_isSpace(str[i]);) {
                token[j++] = str[i++];
            }
            if (strchr(token, '.') == NULL) {
                ff_push(s, (void*)atoll(token));
            }
            else {
                ff_pushf(s, atof(token));
            }
        }
    }
}

void ff_doFile(ffState *s, const char *path) {
    FILE* f = fopen(path, "rb");
    fseek(f, SEEK_SET, SEEK_END);
    long long len = ftell(f);
    rewind(f);

    char* buffer = calloc(len + 1, sizeof(char));
    fread(buffer, 1, len, f);

    buffer[len] = '\0';
    ff_doString(s, buffer);

    free(buffer);
    fclose(f);
}

void ff_registerFunction(ffState* s, const char* name, void (*f)(void)) {
    if (s->fn_top >= s->fn_cap) return; // overflow
    strcpy(s->fn.name[s->fn_top], name);
    s->fn.ptr[s->fn_top] = f;
    s->fn_top++;
}

void ff_clearStack(ffState *s) {
    memset(s->stack, 0, s->cap);
}

void ff_clearMemory(ffState *s) {
    memset(s->memory, 0, s->mem_cap);
}

#endif