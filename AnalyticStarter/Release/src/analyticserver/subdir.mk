################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/analyticserver/AnalyticServerController.cpp 

OBJS += \
./src/analyticserver/AnalyticServerController.o 

CPP_DEPS += \
./src/analyticserver/AnalyticServerController.d 


# Each subdirectory must supply rules for building sources it contributes
src/analyticserver/%.o: ../src/analyticserver/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


