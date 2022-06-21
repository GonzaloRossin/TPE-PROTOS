Protocolos de Comunicacion - TPE
================================

La versión funcional del servidor y cliente SSEMD se encuentra la rama main.

Códigos Fuente
--------------

El código fuente del servidor y el cliente se encuentran dentro de la carpeta server mientras que todos los .h involucrados en el proyecto se encuentran dentro de la carpeta include.

Compilación e Instalación
-------------------------
Para poder utilizar el proyecto es necesario hacer:
```sh
$ make all
```

Este comando va compilar la aplicación servidor y el cliente SSEMD

Configuración
-------------
El servidor y el cliente SSEMD poseen un manfile cada uno dentro de la carpeta de documentación (/notes) y se pueden acceder de esta forma:
```sh
$ man <path_al_archivo>
```
Aquí se pueden ver las configuraciones disponibles para cada una de las aplicaciones que se construyen

Ejecución
---------
Para ejecutar el sevidor se corre el comando
```sh
$ ./socks5d <Opciones>
```
Para ejecutar el cliente SSEMD se corre el comando
```sh
$ ./ssemd <Opciones>
```
Las opciones de cada ejecutable pueden verse utilizando el flag -h

Documentación
-------------
El informe se encuentra en la carpeta "Documentación" bajo el nombre "Informe Protocolos de Comunicación".

Los manfiles conteniendo la información sobre los parámetros que reciben el servidor y el client SSEMD se encuentran también en la carpeta "Documentación" bajo los nombres "socks5d.8" y "ssemd_clientd.8"

Estos archivos pueden verse con el siguiente comando:
```sh
$ man <path_al_archivo>
```
Se incluye también una copia del protocolo desarrollado por nosotros en formato .txt, el mismo se encuentra en la carpeta /Documentacion bajo el nombre "SSEMD_rfc.txt".

Tests
------

En la carpeta "tests" se encuentran archivos de test para algunos de los parsers desarrollados, cada una de las carpetas del parser contiene su propio makefile para poder compilarlo, y para poder ejecutarlo hay que ejecutar el archivo binario que genera el makefile en cada caso

Autores
-------
Grupo 4

Gonzalo Rossin - grossin@itba.edu.ar

Alberto Abancens - aabancens@itba.edu.ar

Uriel Mihura - umihura@itba.edu.ar

Versión
Versión 1.0

1er Cuatrimestre - 2022
