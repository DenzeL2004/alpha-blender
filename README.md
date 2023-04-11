# Альфа блендинг
Работа, созданная в качестве задачи учебного курса Дединского Ильи Рудольфовича.

## Цели 
Познакомится с методами оптимизации. Изучение SIMD инструкций.

## Программа 
Данная программа совмещает две картинки любого размера, но при этом фоновая картинка должна быт по ширине и высоте чем пердняя картинка. Также можно задать смещени картинки по левому верхнему углу.
После выполнения альфа-блендига в окне вы увидите результат работы сомещения.

## Сравнение

Для расчета времени работы была взята картинка 512 * 340 пикселей.

Замер происходил при питание ноутбука от сети.

Каждый пиксель был посчитан 5000 раз. Это было сделано для более точного замера времени выполнения альфа-блендинга. 

В таблице представленно общее время работы альфа-блендинга.

| Версия            | Время сек.   |
| ----------------- | ------------ | 
| Без  оптимизации  | 3.138        |
| AVX2 оптимизации  | 0,428        |

## Вывод 
Из представленных результатов видно, что благодаря использованию AVX команд получилось достигнуть ускорения программы в 7.3 раз.
