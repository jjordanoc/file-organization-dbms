# Proyecto 1 BD2

## Introducción 

Para este proyecto, se nos ha pedido crear y manipular un DataSet usando estrategias de organización de archivos.

### Objetivos
#### Principal
-Conocer el funciomaniento de la manipulación de **metadata** en una base de datos.
#### Secundario
-Identificar las ventajas y desventajas de cada estrategia utilizada. 
-Analizar el comportamiento computacional al implementar una base de datos.
-Comprender como funciona la interacción usuario-servidor mediante la implementación de una GUI.

### Organización de Archivos

Las estrategias usadas para este proyecto son las siguientes:

- ISAM-Sparse Index
- Extendible Hashing

### Funciones implementadas

Para cada estretegia, se debe implementar y visualizar las siguientes funciones:

- vector<Registro> search(T key)
- vector<Registro> rangeSearch(T begin-key, T end-key)
- bool add(Registro registro)
- bool remove(T key)

Además, se pide implementó un **parser** y un GUI en QT para una mejor visualización del proyecto.  

### DataSet
hemos usado un dataset de Películas IMBD. Esta elección fue debida a los siguientes factores:
- Los atributos son fáciles de tratar: la mayoría son de tipo string o int.
- Los datos son requeridos obligatoriamente para todos los atributos menos uno: el "certificate". Esto hace que nuestro potencial de valores NULL disminuya considerablemente.
- El DataSet solo consta de una tabla, por lo que facilita su manejo y manipulación para los índices a utilizar.

A continuación, explicamos los atributos del DataSet previamente mencionado.

```c++
struct MovieRecord {
    int dataId{};
    char contentType[16]{'\0'};
    char title[256]{'\0'};
    short length{};
    short releaseYear{};
    short endYear{};
    int votes{};
    float rating{};
    int gross{};
    char certificate[16]{'\0'};
    char description[512]{'\0'};
```

|       Campo       |                         Descripción                          |
|:-----------------:|:------------------------------------------------------------:|
|   ```dataId```    | Id único de cada registro, sirve para identificar una tupla. |
| ```contentType``` |         Tipo de contenido, puede ser Movie or TvShow         |
|    ```title```    |                     Título del contenido                     |
|   ```length```    |              Longitud del contenido en minutos               |
| ```releaseYear``` |               Año de lanzamiento del contenido               |
|   ```endYear```   |      Finalización del contenido (en caso sea un TvShow)      |
|    ```votes```    |     Cantidad de personas que realizaron una calificación     |
|   ```rating```    |            La calificación promedio del contenido            |
|    ```gross```    |                El ingreso neto del contenido                 |
| ```certificate``` |                 Calificación del certificado                 |
| ```description``` |                  Descripción del contenido                   |
