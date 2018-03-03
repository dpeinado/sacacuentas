################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/platform/deca_mutex.c \
../Src/platform/deca_range_tables.c \
../Src/platform/deca_sleep.c \
../Src/platform/deca_spi.c \
../Src/platform/port.c 

OBJS += \
./Src/platform/deca_mutex.o \
./Src/platform/deca_range_tables.o \
./Src/platform/deca_sleep.o \
./Src/platform/deca_spi.o \
./Src/platform/port.o 

C_DEPS += \
./Src/platform/deca_mutex.d \
./Src/platform/deca_range_tables.d \
./Src/platform/deca_sleep.d \
./Src/platform/deca_spi.d \
./Src/platform/port.d 


# Each subdirectory must supply rules for building sources it contributes
Src/platform/%.o: ../Src/platform/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F103xB -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Inc" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Drivers/STM32F1xx_HAL_Driver/Inc" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Drivers/CMSIS/Include" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Src/application" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Src/decadriver" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Src/compiler" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Src/platform"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


