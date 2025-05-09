# HFT-bot для спотовой торговли для биржи Binance
___
## **Опиасание**:
Данный проект реализует несколько ключевых компонент для торговли на бирже.
В ходе разработке были поставленны цели и задачи, которые были разбиты на маленькие подзадачи.
Для отслеживания выполненной работы использовалась среда Yandex Tracker,
где описывались задачи и назначались ответственные (методология _Agile_).

Данная работа структурирована таким образом, что бы изменяя один компонент, например стратегию торговли, 
не нарушилась работа других модулей

Со структурой файлов можно ознакомиться в блоке ниже. 

Основной язык разработки — _C++  (20 стандарт)_, данный язык был выбран, по ряду причин:

1) Скорость работы языка, является критически важной в данной сфере. С++ позволяет обрабатывать большие объёмы данных
за определённое время. 
2) Так же основой послужило, удобная работа со структурами данных, а именно контейнеры STL. Так в ходе работы
использовались такие контейнеры, как `deque`, `vector`

   
      `deque` - Является самым оптимальной структурой, так как работа идёт с временными данными (удаляем из начала, добавляем в конец)
      
      В двусторонней очереди, вставка происходит со сложностью O(1)
      
      `vector` -  подходит для хранения неизменяемых данных с быстрым доступом по индексу
      
      `map` - Хеш таблица использовалась для хранения API ключей и паролей

3) Данный язык позволяет работать не низком уровне, что идеально подходит для HFT-ботов.
4) Возможна работа с потоками, и обширный простор для оптимизации работы
5) Данный язык позволил реализовать бота, с минимальным потреблением ресурсов

Но данный язык, накладывает на нас ряд **ограничений**:

1) Сложность реализации
2) Использование сторонних библиотек
3) Относительно долгое время компиляции (10-15 с.)


Данный бот поможет, вкладывать средства в активы, и торговать ими, с минимальными рисками. Так как стратегии реализованы, 
что бы иметь минимальные потери

___
## **Сруктура проекта**:



      TBM/
      ├── src/
      │   ├── AccountManager.cpp     // проверка баланса аккаунта
      │   ├── APIread.cpp            // чтение ключей из .env
      │   ├── BinanceAPI.cpp         // подключение к API Binance
      │   ├── DataBase.cpp           // работа с базами данных
      │   ├── INDICATORS/            // реализация индикаторов и стратегий
      │   │   ├── RSI.cpp            // стратегия RSI
      │   │   ├── BB.cpp             // стратегия Bollinger Bands
      │   │   ├── EMA.cpp            // стратегия EMA
      │   │   ├── MACD.cpp           // стратегия MACD
      │   │   └── AbstractStrategy.cpp // базовый класс стратегий
      │   ├── AnalysisHandler.cpp    // обработка анализа рынка
      │   ├── main.cpp               // точка входа
      │   └── OrderManager.cpp       // создание ордеров
      ├── headers/
      │   ├── *.h                    // описания классов и прототипы методов
      │   └── common.h
      ├── utils/
      │   ├── .env                   // переменные окружения
      │   ├── cacert.pem             // SSL-сертификаты
      │   └── *.json                 // вспомогательные файлы
      

___

**Технологический стек**:
- **С++** 20 стандарта
- **STL**
- **BOOST/BEAST BOOST/ASIO** - Для установки соединения и HTTTP запросов
- **nlohman** - для парсинга JSON ответов
- **OpenSSL** - для подписания ордера
- **libpqxx** - для работ с базами данных
- **PostgreSQL** - БД, для логирования данных
- **Docker** - для контейнеризации
- **vcpkg** - для установки библиотек
- **Сmake** - для установки зависимостей
- **Git** - для контроля версий
- **GitHub** - для работы с репозиторием
- **Python 3.12>=** (PyTorch, requests, sklearn)




**Примичание:**

- Используется компилятор MSVS, DCMAKE установить путь на vcpkg
- -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake 
___

## **Сбор данных и их анализ**:

1. **Вебсокет (WebSocket)** 

   Это протокол двусторонней связи в реальном времени, 
   который позволяет получать потоковые данные с биржи Binance
   без необходимости отправлять повторные HTTP-запросы. 
   Это критически важно для трейдинга, где задержки в получении данных могут привести
   к упущенным возможностям или рискам.


- Каждые 1-2 секунды вебсокет передает обновления по ценам, объемам 
и другим параметрам выбранных торговых пар (например, BTC/USDT).

- Минимальная задержка (данные приходят мгновенно).
- Снижение нагрузки на сервер (не нужно постоянно опрашивать API).
- Возможность обрабатывать большие объемы данных в реальном времени.

2. **Анализ данных через индикаторы**

   После сбора данных система передает их в класс AnalysisHandler.cpp, 
   который использует индикаторы для принятия решений.
   Вот ключевые индикаторы и их роль:

- **RSI (Relative Strength Index)**
   Измеряет силу тренда и определяет состояния перекупленности (RSI > 70) 
   или перепроданности (RSI < 30).
  -  Если RSI пересекает **ниже 30** — сигнал к покупке.

      Если RSI **выше 70** — сигнал к продаже.
   
  -  Параметры: обычно используется 14 свечей


- **Bollinger Bands (BB)**
   
   Состоит из трех линий:
   
   Средняя (SMA за 20 периодов).
   
   Верхняя (SMA + 2 стандартных отклонения).
   
   Нижняя (SMA – 2 стандартных отклонения).

  - **Цена у верхней границы** — возможна коррекция вниз.
  
  - **Цена у нижней границы** — возможен рост.

  - "Сужение" полос указывает на низкую волатильность и скорый рывок.

- **MACD (Moving Average Convergence Divergence)**

   Состоит из двух линий:

   MACD Line (разница между 12- и 26-периодной EMA).
   
   Signal Line (9-периодная EMA от MACD Line).

   - Пересечение MACD Line **снизу вверх** Signal Line — сигнал к покупке.
   - Пересечение **сверху вниз** — сигнал к продаже.

- **EMA (Exponential Moving Average)**

  Скользящая средняя, которая придает больший вес последним данным.
  Используются период 200.

Причины выбора этих индикаторов:

RSI и BB — идеальны для определения точек разворота тренда.

MACD — помогает оценить силу тренда и момент входа.

EMA — фильтрует рыночный "шум" и подтверждает тренд.

Комбинация индикаторов снижает риск ложных сигналов. Например:

Покупка только при условии: RSI < 30 + цена у нижней границы BB + MACD выше Signal Line.


____
## **Базы данных**:

Используется PostgreSQL — благодаря своей скорости и простоте.
Также возможно адаптировать код под MongoDB при необходимости.

В базу данных, логируются исторически данные, данные индикаторов каждые n-минут, 
и данные о сделках, в которых описано по какой паре, какое количество и по какой цене
была совершена 
сделка (BUY/SELL). В каждой таблице дополнительно хранится время прихода значений.
___
## **Контейнеризация**:


Каждый модуль (торговый бот, база данных, веб-ресурс) размещён в отдельном контейнере, чтобы изолировать их работу друг от друга.
____
## **Дополнительно**:
Мы рекомендуем использовать пакетный менеджер vcpkg для установления библиотек и зависимостей 

При работе с проектом, получить API ключи на биржу, и задать пароль для БД в `utils/.env`

Если ошибка SSL -  обновить сертификаты, `utils/cacert.pem`


____
**Литература**:

[BinanceAPI](https://developers.binance.com/docs/binance-spot-api-docs)

[Boost/asio](https://www.boost.org/doc/libs/master/doc/html/boost_asio.html)

[Boost/beast](https://www.boost.io/doc/libs/latest/libs/beast/doc/html/index.html)
___
