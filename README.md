# pbps-cours

# Для развёртывания необходимо имень Базу данных postgres

- Для её установки введите

~~~
sudo -u postgres psql postgres
~~~

Для использования библиотек postgres необходимо установить libpq-dev

- Для её установки введите

~~~
sudo apt-get install libpq-dev
~~~

Дальше установим парол для пользователя postgres

- Для этого введём

~~~
sudo -u postgres psql postgres
~~~

получим 

~~~
psql (14.7 (Ubuntu 14.7-0ubuntu0.22.04.1))
Type "help" for help.

postgres=#
~~~

- Теперь вводим команду для установления пароля и вводим сам пароль

~~~
\password postgres
~~~

Управление сервкром postgres

- старт

~~~
sudo service postgresql start
~~~

- стоп

~~~
sudo service postgresql stop
~~~

- статус

~~~
service postgresql status
~~~

Создаём базу данных

- соманда создания

~~~
sudo -u postgres createdb autdb --owner postgres
~~~

Для создания таблицы запускаем из папки postgres программу create_table

- Сборка

~~~
gcc -o create_table create_table.c -I/usr/include/postgresql -lpq -std=c99
~~~

- Запуск

~~~
./create_table
~~~

Для заполнения таблицы запускаем из папки postgres программу select_table

- Сборка

~~~
gcc -o select_table select_table.c -I/usr/include/postgresql -lpq -std=c99
~~~

- Запуск

~~~
./select_table -u [логин] -p [пароль] -a [доступ TRUE/FALSE]
~~~

# Сборка/запуск/остановка проекта

- Сборка

~~~
make 
~~~

- Запуск

~~~
sudo make install
~~~

- Остановка

~~~
sudo make uninstall
~~~
