################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/opencctv/event/AnalyticEvent.cpp \
../src/opencctv/event/EventMessage.cpp \
../src/opencctv/event/EventUtil.cpp \
../src/opencctv/event/ServerEvent.cpp 

OBJS += \
./src/opencctv/event/AnalyticEvent.o \
./src/opencctv/event/EventMessage.o \
./src/opencctv/event/EventUtil.o \
./src/opencctv/event/ServerEvent.o 

CPP_DEPS += \
./src/opencctv/event/AnalyticEvent.d \
./src/opencctv/event/EventMessage.d \
./src/opencctv/event/EventUtil.d \
./src/opencctv/event/ServerEvent.d 


# Each subdirectory must supply rules for building sources it contributes
src/opencctv/event/%.o: ../src/opencctv/event/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


