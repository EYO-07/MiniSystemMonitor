#include "pch.h"
#include "HardwareMonitorWrapper.h"

using namespace System;
using namespace System::Threading; // Necessário para Thread::Sleep
using namespace OpenHardwareMonitor::Hardware;
   
void HardwareMonitorWrapper::update() {
	this->fanInfo = "";
    for each (IHardware ^ hardware in computer->Hardware) {
		hardware->Update();
        if (hardware->HardwareType == HardwareType::CPU) {
            //hardware->Update();
            for each (ISensor ^ sensor in hardware->Sensors) {
                if (sensor->SensorType == SensorType::Load) this->cpu_load = getValue(sensor);
                if (sensor->SensorType == SensorType::Temperature) this->cpu_temp = getValue(sensor);
                if (sensor->SensorType == SensorType::Fan) this->cpu_fan = getValue(sensor);
            }
			//continue;
        }
        if (hardware->HardwareType == HardwareType::GpuNvidia) {
            //hardware->Update();
            for each (ISensor ^ sensor in hardware->Sensors) {
                if (sensor->SensorType == SensorType::Load) this->gpu_load = getValue(sensor);
                if (sensor->SensorType == SensorType::Temperature) this->gpu_temp = getValue(sensor);
                if (sensor->SensorType == SensorType::Fan) this->gpu_fan = getValue(sensor);
            }
			//continue;
        } else if (hardware->HardwareType == HardwareType::GpuAti) {
            //hardware->Update();
            for each (ISensor ^ sensor in hardware->Sensors) {
                if (sensor->SensorType == SensorType::Load) this->gpu_load = getValue(sensor);
                if (sensor->SensorType == SensorType::Temperature) this->gpu_temp = getValue(sensor);
                if (sensor->SensorType == SensorType::Fan) this->gpu_fan = getValue(sensor);
            }
			//continue;
        }
        if (hardware->HardwareType == HardwareType::HDD) {
            //hardware->Update();
            for each (ISensor ^ sensor in hardware->Sensors) {
                if (sensor->SensorType == SensorType::Load) this->drive_load = getValue(sensor);
                if (sensor->SensorType == SensorType::Temperature) this->drive_temp = getValue(sensor);
                if (sensor->SensorType == SensorType::Fan) this->drive_fan = getValue(sensor);
            }
			//continue;
        }
        if (hardware->HardwareType == HardwareType::RAM) {
            //hardware->Update();
            for each (ISensor ^ sensor in hardware->Sensors) {
                if (sensor->SensorType == SensorType::Load) this->ram_load = getValue(sensor);
                if (sensor->SensorType == SensorType::Temperature) this->ram_temp = getValue(sensor);
                if (sensor->SensorType == SensorType::Fan) this->ram_fan = getValue(sensor);
            }
			//continue;
        }
		if (hardware->HardwareType == HardwareType::Mainboard){
			//hardware->Update();
			for each (ISensor ^ sensor in hardware->Sensors) {
				// >>
				if (sensor->SensorType == SensorType::Load) this->mother_load = getValue(sensor);
                if (sensor->SensorType == SensorType::Temperature) this->mother_temp = getValue(sensor);
                if (sensor->SensorType == SensorType::Fan) this->mother_board_fan = getValue(sensor); 
				// <<
            }
			//continue;
		}
		for each (OpenHardwareMonitor::Hardware::ISensor^ sensor in hardware->Sensors) {
            if (sensor->SensorType == SensorType::Fan) {
                this->fanInfo += String::Format("{0:F0} RPM, ", sensor->Value);
            }
        }
	}
}
void HardwareMonitorWrapper::cout() {
    // -- CPU
    std::cout << "[ CPU  ] ";
    if (this->cpu_load) std::cout << this->cpu_load << " \% | ";
    if (this->cpu_temp) std::cout << this->cpu_temp << " Celsius | ";
    if (this->cpu_fan) std::cout << this->cpu_fan << " RPM ";
    std::cout << std::endl;
    // -- GPU
    std::cout << "[ GPU  ] ";
    if (this->gpu_load) std::cout << this->gpu_load << " \% | ";
    if (this->gpu_temp) std::cout << this->gpu_temp << " Celsius | ";
    if (this->gpu_fan) std::cout << this->gpu_fan << " RPM ";
    std::cout << std::endl;
    // -- Drive 
    std::cout << "[ Disk ] ";
    if (this->drive_load) std::cout << this->drive_load << " \% | ";
    if (this->drive_temp) std::cout << this->drive_temp << " Celsius | ";
    if (this->drive_fan) std::cout << this->drive_fan << " RPM ";
    std::cout << std::endl;
    // -- Ram
    std::cout << "[ Ram  ] ";
    if (this->ram_load) std::cout << this->ram_load << " \% | ";
    if (this->ram_temp) std::cout << this->ram_temp << " Celsius | ";
    if (this->ram_fan) std::cout << this->ram_fan << " RPM ";
    std::cout << std::endl;
    // --
    std::cout << std::endl;
    Thread::Sleep(1000); // Pausa por 'intervalMs' milissegundos
}




