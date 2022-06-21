Protocolos de Comunicacion - TPE
================================

La versión funcional del servidor y cliente SSEMD se encuentra la rama main de este repositorio.

Códigos Fuente
--------------

El código fuente del servidor se encuentra dentro de la carpeta server. Este utiliza utilitarios encontrados dentro de la carpeta utils y parsers dentro de la carpeta parsers. Todos sus .h se encuentran dentro de la carpeta include.

El código fuente del cliente del Admin se encuentra dentro de la carpeta admin.


Compilación e Instalación
-------------------------
Para poder utilizar el proyecto es necesario hacer:
```sh
$ make all
```

Este comando compila la aplicación servidor y el cliente SSEMD, dejándolos en la raíz del proyecto.


Documentación
-------------
El informe se encuentra en la carpeta "Documentación".

Los manfiles conteniendo la información sobre los parámetros que reciben el servidor y el client SSEMD se encuentran también en la carpeta "Documentación" bajo los nombres "socks5d.8" y "ssemd_clientd.8"

Estos archivos pueden verse con el siguiente comando:
```sh
$ man <path_al_archivo>
```
Se incluye también una copia del protocolo desarrollado por nosotros en formato .txt, el mismo se encuentra en la carpeta /Documentacion bajo el nombre "SSEMD_rfc.txt".


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


Tests
------

En la carpeta "tests" se encuentran archivos de test para algunos de los parsers desarrollados, cada una de las carpetas del parser contiene su propio makefile para poder compilarlo, y para poder ejecutarlo hay que ejecutar el archivo binario que genera el makefile en cada caso.

Otro test utilizado el archivo bomb, que puede ser generado con
```sh
$ make bomb
```
creando el archivo bomb en la raíz del servidor. Este crea conexiones a la dirección y puerto definidos dentro del bomb.c

Autores
-------
Grupo 4


Alberto Abancens - aabancens@itba.edu.ar

Gonzalo Rossin - grossin@itba.edu.ar

Uriel Mihura - umihura@itba.edu.ar



Versión 1.0

1er Cuatrimestre - 2022
