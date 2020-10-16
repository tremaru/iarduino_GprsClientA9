[![](https://iarduino.ru/img/logo.svg)](https://iarduino.ru)[![](https://wiki.iarduino.ru/img/git-shop.svg?3)](https://iarduino.ru) [![](https://wiki.iarduino.ru/img/git-wiki.svg?2)](https://wiki.iarduino.ru) [![](https://wiki.iarduino.ru/img/git-lesson.svg?2)](https://lesson.iarduino.ru)[![](https://wiki.iarduino.ru/img/git-forum.svg?2)](http://forum.trema.ru)

# iarduino\_GprsModemA9

Библиотека iarduino\_GprsClientA9 позволяет устанавливать TCP и UDP соединения используя несколько простых и понятных функций.

Данная библиотека может использовать как аппаратную, так и программную реализацию шины UART.

## Назначение функций:

\#include **\<GprsModem.h\>** // Подключаем библиотеку iarduino\_GprsClientA9 для работы с GSM/GPRS Shield.

### Функции объекта модема

GprsModem **ОБЪЕКТ\_МОДЕМА****(** UART **);** // Создаём объект для работы с функциями модема.

Функция **ОБЪЕКТ\_МОДЕМА**.**begin();** // Поиск и конфигурация модема на шине UART

Функция **ОБЪЕКТ\_МОДЕМА**.**coldReboot(** \[ВЫВОД\] **)**  // Холодная перезагрузка, ВЫВОД - вывод PWR Shield'а. По умолчанию 9.

Функция **ОБЪЕКТ\_МОДЕМА**.**getSignalLevel();** // Получение качества приёма сигнала

### Функции объекта клиета


GprsClient **ОБЪЕКТ\_КЛИЕНТА****(** UART **);** // Создаём объект для работы с функциями клиента

Функция **ОБЪЕКТ\_КЛИЕНТА**.**begin();** // Перевод модема в режим работы с Gprs.

Класс клиента является дочерним класса **Client**, поэтому наследует все публичные методы этого класса.

Функция **ОБЪЕКТ\_КЛИЕНТА**.**connect(** УЗЕЛ\_СЕТИ, ПОРТ, \[ПРОТОКОЛ\] **);** // Соединение удалённым узлом

Функция **ОБЪЕКТ\_КЛИЕНТА**.**println(** СТРОКА **);** // Отправка строки запроса на подключённый удалённый узел (унаследована от объекта Stream, полностью идентична функции Serial.println()

Функция **ОБЪЕКТ\_КЛИЕНТА**.**available();** // Возвращает кол-во байт, доступных для чтения. В случае отсутствия данных возвращает -1

Функция **ОБЪЕКТ\_КЛИЕНТА**.**stop();** // Отключение от удалённого узла.
