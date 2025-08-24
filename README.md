## Note: This project was developed during our __third__ year of the Computer Engineering program at the University of Buenos Aires.
_This repository **does not** reflect our current programming level or professional skills. It is kept here as an academic and knowledge record._

------------------------------------------------------------------------------------

**README** available in English and Spanish | **README** disponible en Inglés y en Español

# fisopfs — FUSE-based Userspace Filesystem

This repository contains a `FUSE-style` userspace filesystem developed for the **Operating Systems (7508)** course at **FIUBA (Méndez–Fresia)** _(https://fisop.github.io/website/tps/filesystem)_. It mounts a test directory and persists the filesystem state to a single on-disk file, making it easy to build, run, and experiment locally or inside Docker.

> Tech overview: primarily Python with some C components; build and run are driven by a Makefile.  
> Requires a Linux environment with FUSE available.


## Theoretical answers

Use the `fisopfs.md` file provided in the repository.

## Compile

```bash
$ make
```

## Run

### Setup

First, create a test directory:

```bash
$ mkdir prueba
```

### Start the FUSE server

From the same directory where the solution was compiled, execute:

```bash
$ ./fisopfs prueba/
```

There is a `--filedisk NAME` flag to specify which file should be used
as the on-disk persistence file. The default is "persistence_file.fisopfs":

```bash
$ ./fisopfs prueba/ --filedisk nuevo_disco.fisopfs
```

### Verify mount

```bash
$ mount | grep fisopfs
```

### Use the "test" directory

Open another terminal and execute:

```bash
$ cd prueba
$ ls -al
```

### Cleanup

```bash
$ sudo umount prueba
```

## Docker

There are three _targets_ in the `Makefile` to work with _Docker_:
- `docker-build`: builds an "Ubuntu 20.04" image with FUSE dependencies.
- `docker-run`: creates a _container_ based on the previous image and runs `bash`.
   - here you can run `make` and then `./fisopfs -f ./prueba`.
- `docker-attach`: attach to the same previous container for iterative testing.
   - here you can navigate to the `prueba` directory inside.

## Linter

```bash
$ make format
```

To commit the formatting changes:

```bash
$ git add .
$ git commit -m "format: apply code formatter"
```



------------------------------------------------------------------------------------

## Nota: Este proyecto fue desarrollado durante nuestro **tercer** año del programa de Ingeniería en Informática en la Universidad de Buenos Aires.
_Este repositorio **no** refleja nuestro nivel actual de programación ni nuestro set de skills profesionales. Se guarda aquí como un registro académico y de conocimiento.__

# fisopfs — Sistema de archivos tipo FUSE

Este repositorio contiene un sistema de archivos tipo `FUSE` desarrollado para la materia **Sistemas Operativos (7508) en FIUBA (Méndez–Fresia)** _(https://fisop.github.io/website/tps/filesystem)_. Monta un directorio de prueba y persiste el estado del sistema de archivos en un único archivo en disco, lo que facilita compilar, ejecutar y experimentar localmente o dentro de Docker.

> Descripción técnica: principalmente Python con algunos componentes en C; la compilación y ejecución se realizan mediante un Makefile.
> Requiere un entorno Linux con FUSE disponible.


## Respuestas teóricas

Utilizar el archivo `fisopfs.md` provisto en el repositorio

## Compilar

```bash
$ make
```

## Ejecutar

### Setup

Primero hay que crear un directorio de prueba:

```bash
$ mkdir prueba
```

### Iniciar el servidor FUSE

En el mismo directorio que se utilizó para compilar la solución, ejectuar:

```bash
$ ./fisopfs prueba/
```

Hay una flag `--filedisk NAME` para indicar que archivo se
 quiere utilizar como archivo de persistencia en disco. 
 El valor por defecto es "persistence_file.fisopfs"

```bash
$ ./fisopfs prueba/ --filedisk nuevo_disco.fisopfs
```

### Verificar directorio

```bash
$ mount | grep fisopfs
```

### Utilizar el directorio de "pruebas"

En otra terminal, ejecutar:

```bash
$ cd prueba
$ ls -al
```

### Limpieza

```bash
$ sudo umount prueba
```

## Docker

Existen tres _targets_ en el archivo `Makefile` para utilizar _docker_.

- `docker-build`: genera la imagen basada en "Ubuntu 20.04" con las dependencias de FUSE
- `docker-run`: crea un _container_ basado en la imagen anterior ejecutando `bash`
   - acá se puede ejecutar `make` y luego `./fisopfs -f ./prueba`
- `docker-attach`: permite vincularse al mismo _container_ anterior para poder realizar pruebas
   - acá se puede ingresar al directorio `prueba`

## Linter

```bash
$ make format
```

Para efectivamente subir los cambios producidos por el `format`, hay que usar `git add .` y `git commit`.
