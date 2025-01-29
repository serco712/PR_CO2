##  ------ LICENSE ---------------------------------------------------------------------------------------
##  This application is under the license: GPL
##  --------------------------------------------------------------------------------------------------------

| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

##  Inicialización del menukonfig

-IMPORTANTE: en el menukonfig en la sección de Compiler options, concretamente en la de Optimization Level cambiar la opción que esta por Optimize for size (-Os)

## Como hacer el provisionamiento wifi

-Nos dirigimos al directorio esp-idf/tools/esp_prov
-Creamos un entorno virtual con el comando source .venv/bin/activate
-Hacemos un export IDF_PATH e instalamos las dependencias que marca el README.md
-Una vez que la placa este lista para ser provisionada nos conectamos al wifi de la placa y ejecutamos el comando 
python3 esp_prov.py --transport softap --sec2_username wifiprov --sec2_pwd abcd1234 --ssid Redmi9A --sec_ver 2  --passphrase uddhjdk623 --custom_data "{\"URI\":\"mqtt://thingsboard.cloud:1883\",\"deviceSecret\":\"5o9qixyvornylkfcp5kb\",\"deviceKey\":\"2nin4453393nseh1c78x\",\"jerarquia\":\"UCM/Info/Piso1/Aula/1/N\"}"

-Este comando provisiona wifi y una serie de valores en la opción de custom_data los cuales son: la URI de la pagina de thingsboard o de su ubicación en docker, el device secret y el device key para conectarse al perfil de dispositivo y la posición de la jerarquia donde esta la cual esta ordenada por entidad(UCM)/Edificio(Info)/Piso(Piso1)/Tipo de habitación(Aula, Lab o Despacho)/Número de la habitación(1,2,3...)/ubicación geografica(N, S, E o O)

## Funcionamiento

-Una vez provisionado el wifi se guardara todo en la NVS, por lo que en futuros nuevos arranques siempre que no se borre la flash tendra todos los valores de wifi y de custom_data guardados.
-En cuanto se conecte a la red wifi y esta inicializado mqtt el dispositivo se inicializa en thingsboard creando el dispositivo en el perfil que queremos y con un atributo cliente que contiene su jerarquia. 
-Una vez hecho esto se empezarán a enviaran datos de manera periodica del sensor sgp30 al dispositivo recien creado. 

## Distribución de archivos

El proyecto **PR_CO2** contiene un archivo principal en lenguaje C [main.c](main/main.c). El archivo se localiza en la carpeta [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

A continuación se muestra la disposición de carpetas y archivos:

```
├── components
│   ├── mqtt_comp
|   |   ├── inlcude
|   |   |   └── mqtt_sensor.h
|   │   ├── CMakeLists.txt
|   │   ├── component.mk
|   │   └── mqtt_sesnor.c
|   ├── ota
|   |   ├── inlcude
|   |   |   └── ota.h
|   │   ├── CMakeLists.txt
|   │   ├── component.mk
|   │   └── ota.c
|   ├── sgp30_comp
|   |   ├── inlcude
|   |   |   └── sgp30.h
|   │   ├── CMakeLists.txt
|   │   ├── component.mk
|   │   └── sgp30.c  
├── main
│   ├── CMakeLists.txt
│   ├── deep_sleep.c
│   ├── idf_component.yml
│   ├── main.c
│   ├── wifi.c
│   └── wifi.h
├── CMakeLists.txt
└── README.md                  This is the file you are currently reading
```

## Enlace al repositorio GitHUb

https://github.com/serco712/PR_CO2