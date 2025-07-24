# ESP32

## Activate ESP_IDF

- Make it executable

```bash
    chmod +x ./activate-esp-idf.sh
```

- Run the shell

```bash
    source ./activate-esp-idf.sh
```

## Build the project

```bash
    idf.py build
```

## Flash the compiled firmware onto ESP32

    - connect esp32 to your computer via USB
    - Identity the serial port:

```bash
  ls /dev/tty.*
```

    - Flash the firmware

```bash
idf.py -p /dev/tty.usbserial-0001 flash
```

    - Monitor the serial output

```bash
idf.py -p /dev/tty.usbserial-0001 monitor
```

## Folder contents

The project **hello_world** contains one source file in C language [hello_world_main.c](main/hello_world_main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both).

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── pytest_hello_world.py      Python script used for automated testing
├── main
│   ├── CMakeLists.txt
│   └── hello_world_main.c
└── README.md                  This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting

- Program upload failure

  - Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
  - The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.
