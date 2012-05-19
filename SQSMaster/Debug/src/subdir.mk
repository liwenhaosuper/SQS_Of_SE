################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Logger.cpp \
../src/MsgLock.cpp \
../src/SQSMaster.cpp \
../src/main.cpp 

OBJS += \
./src/Logger.o \
./src/MsgLock.o \
./src/SQSMaster.o \
./src/main.o 

CPP_DEPS += \
./src/Logger.d \
./src/MsgLock.d \
./src/SQSMaster.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I../../libevent-2.0.19/include/ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


