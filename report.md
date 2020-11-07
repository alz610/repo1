# 2020-11-07

c_4.1.log -- выводы 20 запусков программы `test1.c` с 1 потоком парсинга чанка (`n_parsethreads = 1` на 153 строке `parse.c`):
  - во всех запусках программа зависла;

c_4.2.log -- выводы 20 запусков программы `test1.c` с 2 потоками парсинга чанка (`n_parsethreads = 2` на 153 строке `parse.c`):
  - в 12 запусках программа зависла.
  - в 2 запусках провалился ассерт:
    - ``test1.out: parse.c:97: parsechunk: Assertion `cols == n_parsed_nums_in_line' failed.``
  - в остальных запусках завершилась работа с правильно распарсенным массивом 

c_4.3.log -- выводы 24 запусков программы `test1.c` с 3 потоками парсинга чанка (`n_parsethreads = 3` на 153 строке `parse.c`):
  - в 2 запусках программа зависла;
  - в 7 запусках провалился ассерт:
    - ``test1.out: parse.c:97: parsechunk: Assertion `cols == n_parsed_nums_in_line' failed.``
  - в остальных запусках завершилась работа с правильно распарсенным массивом


# 2020-11-06

c_3.1.log -- выводы 7 запусков программы `main.c` с 1 потоком парсинга чанка (`n_parsethreads = 1` на 124 строке `parse.c`):
  - во всех запусках программа зависла;

c_3.2.log -- выводы 7 запусков программы `main.c` с 2 потоками парсинга чанка (`n_parsethreads = 2` на 124 строке `parse.c`):
  - в 4 запусках программа зависла.
  - в остальных запусках:
    - массив `arr` частично совпадает с ожидаемым массивом, т.е. не все значения чисел в массиве `arr` совпадают с точными значениями чисел;
    - подчанки распарсены правильно, следовательно неверно записаны в массив `arr`.

c_3.3.log -- выводы 7 запусков программы `main.c` с 3 потоками парсинга чанка (`n_parsethreads = 3` на 124 строке `parse.c`):
  - в 2 запусках программа зависла;
  - в одном запуске провалился ассерт ``main.out: parse.c:91: parsechunk: Assertion `cols == _i' failed``;
  - в остальных запусках:
    - массив `arr` частично совпадает с ожидаемым массивом;
    - подчанки распарсены правильно.