import numpy as np
from ctypes import *
from timeit import default_timer as timer


def load_sys_C_lib():
    from ctypes.util import find_library

    # загрузка библиотеки системы Linux
    libc = CDLL(find_library("c"))

    fopen = libc.fopen
    fopen.argtypes = c_char_p, c_char_p
    fopen.restype = c_void_p

    fwrite = libc.fwrite
    fwrite.argtypes = c_void_p, c_size_t, c_size_t, c_void_p
    fwrite.restype = c_size_t

    fclose = libc.fclose
    fclose.argtypes = c_void_p,
    fclose.restype = c_int

    fmemopen = libc.fmemopen
    fmemopen.argtypes = c_void_p, c_size_t, c_char_p
    fmemopen.restype = c_void_p

    return libc


def load_parse_lib():
    # загрузка библиотеки parse
    libparse = CDLL('./libparse.so')

    parsefile = libparse.parsefile
    parsefile.argtypes = POINTER(c_float), c_size_t, c_size_t, c_void_p
    parsefile.restype = c_size_t

    return libparse


def test0(arrsize, n, m):
    '''
    Парсинг файла fp в массив чисел arr.

        arrsize -- длина массива распарсенных чисел (в числах)
        n -- длина строк чанка (в символах)
        m -- длина чанка (в строках)
    '''

    # загрузка библиотек си
    libc = load_sys_C_lib()
    libparse = load_parse_lib()


    fp = libc.fopen(b'data.txt', b'r')  # читаемый текст
    arr = (c_float * arrsize)()         # записываемый массив


    st = timer()

    nread = libparse.parsefile(arr, n, m, fp)

    total = timer() - st


    print("total time: {:f} ms".format (total * 1000))
    print("total floats read: {:d}".format (nread))

    # сравнение массива распарсенных чисел с массивом известных чисел
    arr_exact = np.load('data.npy')
    print("check: {}".format (np.allclose (arr, arr_exact)))

    # print(np.array(arr), '\n')


    libc.fclose(fp)


def test1(arr_exact, arr_str):
    '''
    Парсинг строки arr_str в массив чисел arr.
    '''

    libc = load_sys_C_lib()
    libparse = load_parse_lib()


    fp = libc.fmemopen(arr_str, len(arr_str), b"r")
    arr = (c_float * arrsize)()         # записываемый массив


    st = timer()

    nread = libparse.parsefile(arr, n, m, fp)

    total = timer() - st


    print("total time: {:f} ms".format (total * 1000))
    print("total floats read: {:d}".format (nread))

    # сравнение массива распарсенных чисел с массивом известных чисел
    arr_exact = np.load('data.npy')
    print("check: {}".format (np.allclose (arr, arr_exact)))

    # print(np.array(arr1), '\n')

    libc.fclose(fp)


def gen_data0(arrsize, cols):
    '''
    Генерация данных для парсинга.
    Записывает в носитель информации.
    '''

    # генерация массива
    arr = 10 ** np.random.uniform(-3, 3, arrsize)

    # запись массива с известными числами в носитель
    np.save('data.npy', arr)
    # запись текстовика с cols колонками в носитель
    np.savetxt('data.txt', arr.reshape(arrsize // cols, cols))


def gen_data1(arrsize, cols):
    '''
    Генерация данных для парсинга.
    Не записывает в носитель информации.
    '''

    import io

    arr = 10 ** np.random.uniform(-3, 3, arrsize)

    with io.BytesIO() as bio:
        np.savetxt(bio, arr.reshape(arrsize // cols, cols))  # write to the fake file
        arr_str = bio.getvalue()  # read the fake file

    return arr, arr_str


def main0():
    from os.path import isfile


    cols = 5            # длина строки текста (в числах)
    arrsize = 100000000 # длина массива распарсенных чисел (в числах)
    m_ = 100000         # длина чанка (в числах)


    # если текстового файла не существует
    if not isfile('data.txt'):
        # если массив с известными числами не существует или его длина не равна arrsize
        if (not isfile('data.npy')) or (np.load('data.npy').shape != (arrsize,)):
            print('generating data...\n\n')
            gen_data0(arrsize, cols)


    print("parallel program\n")
    test0(arrsize, n=128, m=m_ // cols)


    print("\n\nnonparallel program\n")
    test0(arrsize, n=128, m=arrsize // cols)


def main1():
    arrsize = 10000000


    print('generating data...\n\n')

    arr, arr_str = gen_data1(arrsize, cols=5)


    print("parallel program\n")

    test1(arr, arr_str)


if __name__ == "__main__":
    main0()