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


'''
Парсинг файла fp в массив чисел arr.
'''
def test0():
    # загрузка библиотек си
    libc = load_sys_C_lib()
    libparse = load_parse_lib()


    cols = 5            # длина строки текста (в числах)
    arrsize = 1000000   # длина массива распарсенных чисел (в числах)
    m_ = 100000         # длина чанка (в числах)

    n = 128             # длина строк чанка (в символах)
    m = m_ // cols      # длина чанка (в строках)


    fp = libc.fopen(b'data.txt', b'r')  # читаемый текст
    arr = (c_float * arrsize)()         # записываемый массив


    st = timer()

    nread = libparse.parsefile(arr, n, m, fp)

    total = timer() - st


    print("total time: {:f} ms".format (total * 1000))
    print("total floats read: {:d}".format (nread))

    # сравнение массива распарсенных чисел с массивом известных чисел
    arr_exact = np.load('data.npy')
    print("integrity: {}".format (np.allclose (arr, arr_exact)))

    # print(np.array(arr), '\n')


    libc.fclose(fp)


'''
Парсинг файла fp в массив чисел arr.
Непаралелльная версия.
Отличается от параллельной программы длиной чанка (в числах),
который равен длине записываемого массива чисел arr.
'''
def test0_nonparallel():
    # загрузка библиотек си
    libc = load_sys_C_lib()
    libparse = load_parse_lib()


    cols = 5            # длина строки текста (в числах)
    arrsize = 1000000   # длина массива распарсенных чисел (в числах)
    m_ = arrsize        # длина чанка (в числах)

    n = 128             # длина строк чанка (в символах)
    m = m_ // cols      # длина чанка (в строках)


    fp = libc.fopen(b'data.txt', b'r')  # читаемый текст
    arr = (c_float * arrsize)()         # записываемый массив


    st = timer()

    nread = libparse.parsefile(arr, n, m, fp)

    total = timer() - st


    print("total time: {:f} ms".format (total * 1000))
    print("total floats read: {:d}".format (nread))

    # сравнение массива распарсенных чисел с массивом известных чисел
    arr_exact = np.load('data.npy')
    print("integrity: {}".format (np.allclose (arr, arr_exact)))

    # print(np.array(arr), '\n')


    libc.fclose(fp)


'''
Парсинг строки arr_str в массив чисел arr.
'''
def test1(arr_exact, arr_str):
    libc = load_sys_C_lib()
    libparse = load_parse_lib()


    cols = 5            # длина строки текста (в числах)
    arrsize = 1000000   # длина массива распарсенных чисел (в числах)
    m_ = 100000         # длина чанка (в числах)

    n = 128             # длина строк чанка (в символах)
    m = m_ // cols      # длина чанка (в строках)


    fp = libc.fmemopen(arr_str, len(arr_str), b"r")
    arr = (c_float * arrsize)()         # записываемый массив


    st = timer()

    nread = libparse.parsefile(arr, n, m, fp)

    total = timer() - st


    print("total time: {:f} ms".format (total * 1000))
    print("total floats read: {:d}".format (nread))
    print("integrity: {}".format (np.allclose (arr, arr_exact)))

    # print(np.array(arr1), '\n')

    libc.fclose(fp)


'''
Генерация данных для парсинга.
Записывает в носитель информации.
'''
def gen_data0():
    from os.path import isfile

    # если оба файла существуют
    if isfile('data.npy') and isfile('data.txt'):
        return


    cols = 5
    arrsize = 1000000

    # генерация массива
    arr = 10 ** np.random.uniform(-3, 3, arrsize)

    # запись массива с известными числами в носитель
    np.save('data.npy', arr)
    # запись текстовика с cols колонками в носитель
    np.savetxt('data.txt', arr.reshape(arrsize // cols, cols))


'''
Генерация данных для парсинга.
Не записывает в носитель информации.
'''
def gen_data1():
    import io

    cols = 5
    arrsize = 1000000

    arr = 10 ** np.random.uniform(-3, 3, arrsize)

    with io.BytesIO() as bio:
        np.savetxt(bio, arr.reshape(arrsize // cols, cols))  # write to the fake file
        arr_str = bio.getvalue()  # read the fake file
    
    return arr, arr_str


if __name__ == "__main__":
    gen_data0()

    print("parallel program\n")
    test0()

    print("\n\nnonparallel program\n")
    test0_nonparallel()

    # test1(*gen_data1())