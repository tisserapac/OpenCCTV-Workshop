################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/test/gateway/subdir.mk
-include src/opencctv/util/serialization/gpb/subdir.mk
-include src/opencctv/util/serialization/subdir.mk
-include src/opencctv/util/log/colored/subdir.mk
-include src/opencctv/util/log/subdir.mk
-include src/opencctv/util/flow/subdir.mk
-include src/opencctv/util/subdir.mk
-include src/opencctv/mq/subdir.mk
-include src/opencctv/event/subdir.mk
-include src/opencctv/dto/subdir.mk
-include src/opencctv/db/subdir.mk
-include src/opencctv/subdir.mk
-include src/analytic/xml/subdir.mk
-include src/analytic/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: OpenCCTVServer

# Tool invocations
OpenCCTVServer: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L/usr/local/lib -pthread -o "OpenCCTVServer" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) OpenCCTVServer
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
