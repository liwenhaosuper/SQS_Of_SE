################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/DataNode.cpp \
../src/Database.cpp \
../src/Logger.cpp \
../src/main.cpp 

OBJS += \
./src/DataNode.o \
./src/Database.o \
./src/Logger.o \
./src/main.o 

CPP_DEPS += \
./src/DataNode.d \
./src/Database.d \
./src/Logger.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../libevent-2.0.19/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


