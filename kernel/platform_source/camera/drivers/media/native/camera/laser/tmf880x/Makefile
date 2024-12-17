ccflags-y += -I$(srctree)/platform_source/camera/drivers/media/native/camera/laser/tmf880x/tof8801/include
ccflags-y += -I$(src)/tof8801

obj-y += tof8801.o

# module
tof8801-objs := laser_tof8801_module.o

# tof8801
tof8801-objs += tof8801/tof_hex_interpreter.o 
tof8801-objs += tof8801/tof8801_app0.o 
tof8801-objs += tof8801/tof8801_bootloader.o 
tof8801-objs += tof8801/tof8801_driver.o
tof8801-objs += tof8801/tof8801_bootloader.o
tof8801-objs += tof8801/tof8801_persistence_filter.o
