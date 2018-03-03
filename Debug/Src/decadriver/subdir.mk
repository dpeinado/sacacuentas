################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/decadriver/deca_device.c \
../Src/decadriver/deca_params_init.c 

OBJS += \
./Src/decadriver/deca_device.o \
./Src/decadriver/deca_params_init.o 

C_DEPS += \
./Src/decadriver/deca_device.d \
./Src/decadriver/deca_params_init.d 


# Each subdirectory must supply rules for building sources it contributes
Src/decadriver/%.o: ../Src/decadriver/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F103xB -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Inc" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Drivers/STM32F1xx_HAL_Driver/Inc" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Drivers/CMSIS/Include" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Src/application" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Src/decadriver" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Src/compiler" -I"C:/Users/dpeinado/Google Drive/04 Pentinard/BigMeetingPoint/workspace/sp_tx_interrupts/Src/platform"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


