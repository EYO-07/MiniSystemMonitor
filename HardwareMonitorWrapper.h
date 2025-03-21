#pragma once
#include "pch.h"
#include <iostream>
#include <vector>

using namespace System;
using namespace System::Threading; // Necessário para Thread::Sleep
using namespace OpenHardwareMonitor::Hardware;

public ref class HardwareMonitorWrapper {
private:
    Computer^ computer; // Substitua msclr::auto_gcroot por um handle gerenciado (^)
public:
    // -- cpu
    double cpu_load;
    double cpu_temp;
    double cpu_fan;
    // -- gpu
    double gpu_load;
    double gpu_temp;
    double gpu_fan;
    // -- drive
    double drive_load;
    double drive_temp;
    double drive_fan;
    // -- ram
    double ram_load;
    double ram_temp;
    double ram_fan;

    HardwareMonitorWrapper() {
        // --
        cpu_temp = 0.0;
        cpu_load = 0.0;
        cpu_fan = 0.0;
        // --
        gpu_temp = 0.0;
        gpu_fan = 0.0;
        gpu_load = 0.0;
        // --
        drive_temp = 0.0;
        drive_fan = 0.0;
        drive_load = 0.0;
        // --
        ram_load = 0.0;
        ram_temp = 0.0;
        ram_fan = 0.0;
        // --
        computer = gcnew Computer();
        computer->CPUEnabled = true;
        computer->GPUEnabled = true;
        computer->HDDEnabled = true;
        computer->RAMEnabled = true;
        //computer->MainboardEnabled = true;
        computer->Open();
    }

    double getValue(ISensor^ sensor) { return sensor->Value.HasValue ? sensor->Value.Value : 0.0; }
    void update();
    void cout();
    ~HardwareMonitorWrapper() {
        computer->Close();
    }
};