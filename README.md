# FanyForth
- Hafif Gömülebilir Programlama Dili
- Typeless Stack Tabanlı Sanal Makineye Sahip
- Çalışması için libc yeterli
- Opsiyonel Memory Yapısı
- C fonksiyon kaydı ve çağrısı yapılabilir

## Örnek

```
( MERAK ETMEYIN BU YORUM SATIRIDIR, CALISTIRILMIYOR )

25.0    ( STACK: 25.0           MEMORY: )
dup     ( STACK: 25.0 25.0      MEMORY: )
rsq     ( STACK: 25.0 0.199690  MEMORY: )
fmul    ( STACK: 4.992238       MEMORY: )
0 stt   ( STACK:                MEMORY: [0]: 4.992238 )
5 12    ( STACK: 5 12           MEMORY: [0]: 4.992238 )
mul     ( STACK: 60             MEMORY: [0]: 4.992238 )
0 ldt   ( STACK: 60 4.992238    MEMORY: )
fmul    ( STACK: 300            MEMORY: )

'Hello,_World! ( ' String tanımlar, _ karakteri boşluk olarak okunur. )

@print_top ( C TARAFINDAN YOLLADIGIN FONKSIYONLAR ICIN @ KULLANIYORUM )
@print_str ( ISTEDIGIN GIBI CESITLENDIR )
```

```c
#include <stdio.h>
#define FFVM_IMPL   // BU DOSYADA FONKSIYONLARIN YAZILMASI ICIN
#define FFVM_DEBUG  // INFOLARI GORMEK ICIN
#define FF_MEMORYCELL_MAX 32    // SINIR BELIRLEMEK ICIN
#define FF_CALLBACK_MAX 64      // AYNI SEKILDE
#include "fanyforth/ff.h"

ffState* vm;

void reg_print_top() {
    printf("%d \n", ff_pop(vm));
}

void reg_print_str() {
    printf("STRING: %s\n", ff_getString(vm));
}

int main() {
    vm = ff_init(32);

    ff_registerFunction(vm, "print_top", reg_print_top);
    ff_registerFunction(vm, "print_str", reg_print_str);
    ff_doFile(vm, "test.ff");

    ff_doString(vm, "1");
    for (int i = 0; i < 5; i++) {
        ff_doFile(vm, "double_it.ff"); // dup add dup @print_top
        // out: 2 4 8 16 32
    }

    ff_deinit(vm);
    return 0;
}
```