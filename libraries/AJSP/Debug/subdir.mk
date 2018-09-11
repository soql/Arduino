################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../AJSP.cpp \
../PathConstructor.cpp \
../PathListener.cpp \
../PathPrinter.cpp \
../Printer.cpp \
../main.cpp 

OBJS += \
./AJSP.o \
./PathConstructor.o \
./PathListener.o \
./PathPrinter.o \
./Printer.o \
./main.o 

CPP_DEPS += \
./AJSP.d \
./PathConstructor.d \
./PathListener.d \
./PathPrinter.d \
./Printer.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


