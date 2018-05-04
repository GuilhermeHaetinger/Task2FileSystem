# **Operating System First Task**

First Task of the Operating System 1 Course from UFRGS

## **Getting Started**

**Debugging**
```
env LGA_LOGGER=LGA_LOGGER_OPTIONS
```

**Test logs**
```
env LGA_LOGGER_TEST=LGA_LOGGER_TEST_OPTIONS
```

**Important logs**
```
env LGA_LOGGER_IMPORTANT=LGA_LOGGER_IMPORTANT_OPTIONS
```
Go to **testes/** directory and run **make "your-test"**, then go to **build/** and run the executable file with the environment variables that you want.


## **LGA_LOGGER_OPTIONS**
* LGA_UNABLE  = 0
* LGA_ERROR  = 1
* LGA_WARNING  = 2
* LGA_LOG  = 3

## **LGA_LOGGER_TEST_OPTIONS**
* LGA_UNABLE  = 0
* LGA_ENABLE  = 1

## **LGA_LOGGER_IMPORTANT_OPTIONS**
* LGA_UNABLE  = 0
* LGA_ENABLE  = 1


## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
