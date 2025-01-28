| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# _Sample project_

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This is the simplest buildable example. The example is used by command `idf.py create-project`
that copies the project to user specified path and set it's name. For more information follow the [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)



## Como hacer el provisionamiento wifi

-Nos dirigimos al directorio esp-idf/tools/esp_prov
-Creamos un entorno virtual con el comando source .venv/bin/activate
-Hacemos un export IDF_PATH e instalamos las dependencias que marca el README.md
-Una vez que la placa este lista para ser provisionada nos conectamos al wifi de la placa y ejecutamos el comando 
python3 esp_prov.py --transport softap --sec2_username wifiprov --sec2_pwd abcd1234 --ssid Redmi9A --sec_ver 2  --passphrase uddhjdk623 --custom_data "{\"URI\":\"mqtt://192.168.8.214:1883\",\"deviceSecret\":\"exmrrodm40frz7wjje3d\",\"deviceKey\":\"ja47czbn3npxz0tn8hal\",\"jerarquia\":\"UCM/Info/Piso1/Aula1/N\"}"

## Example folder contents

The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── components
│   ├── mqtt_comp
|   |   ├── inlcude
|   |   |   └── mqtt_sensor.h
|   │   ├── CMakeLists.txt
|   │   ├── component.mk
|   │   └── mqtt_sesnor.c
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
Additionally, the sample project contains Makefile and component.mk files, used for the legacy Make based build system. 
They are not used or needed when building with CMake and idf.py.
