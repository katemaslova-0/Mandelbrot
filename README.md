# Результаты измерений времени расчёта точек множества Мандельброта

## Системные характеристики
<p style="font-size: 20px;">Процессор: AMD Ryzen™ 7 7435HS, 3.1 Ггц</p>
<p style="font-size: 20px;">Версия компилятора: g++ 13.3.0</p>
<p style="font-size: 20px;">Поддержка AVX2 инструкций</p>

## Версия 1: базовая
<table style="width: 70%; border-collapse: collapse; font-size: 20px;">
  <tr>
    <th>-O0, 10^-6 тик/ит</th>
    <th>-O1, 10^-6 тик/ит</th>
    <th>-O2, 10^-6 тик/ит</th>
    <th>-O3, 10^-6 тик/ит</th>
  </tr>
  <tr>
    <td>729.02 +- 0.10</td>
    <td>261.04 +- 0.09</td>
    <td>256.35 +- 0.06</td>
    <td>256.28 +- 0.04</td>
  </tr>
</table>

## Версия 2: на массивах
<table style="width: 70%; border-collapse: collapse; font-size: 20px;">
  <tr>
    <th>-O0, 10^-6 тик/ит</th>
    <th>-O1, 10^-6 тик/ит</th>
    <th>-O2, 10^-6 тик/ит</th>
    <th>-O3, 10^-6 тик/ит</th>
  </tr>
  <tr>
    <td>1603 +- 10</td>
    <td>382.37 +- 0.15</td>
    <td>241.33 +- 0.04</td>
    <td>126.01 +- 0.03</td>
  </tr>
</table>


## Версия 3: c использованием AVX2 инструкций
<table style="width: 70%; border-collapse: collapse; font-size: 20px;">
  <tr>
    <th>-O0, 10^-6 тик/ит</th>
    <th>-O1, 10^-6 тик/ит</th>
    <th>-O2, 10^-6 тик/ит</th>
    <th>-O3, 10^-6 тик/ит</th>
  </tr>
  <tr>
    <td>285.15 +- 0.03</td>
    <td>35.03 +- 0.01</td>
    <td>34.91 +- 0.01</td>
    <td>34.90 +- 0.01</td>
  </tr>
</table>

## Сравнение времени исполнения (с -O3)
<table style="width: 70%; border-collapse: collapse; font-size: 20px;">
  <tr>
    <th>Версия 1</th>
    <th>Версия 2</th>
    <th>Версия 3</th>
  </tr>
  <tr>
    <td>x</td>
    <td>2.0x</td>
    <td>7.3x</td>
  </tr>
</table>

## Результат
<p style="font-size: 20px;">Получаем ускорение относительно первоначальной версии примерно в 7.3 раз.</p>
