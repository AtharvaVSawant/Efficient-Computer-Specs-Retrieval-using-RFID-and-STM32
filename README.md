# Efficient-Computer-Specs-Retrieval-using-RFID-and-STM32
Automated computer specification retrieval system using RFID and STM32 microcontroller. Scan RFID tags attached to computer components to instantly display detailed hardware specifications on the LCD. Features IoT integration with NodeMCU for cloud data upload.


# 🖥️ Efficient Computer Specifications Retrieval Using RFID and STM32

## 🎯 Project Overview

This project implements an **automated computer specification retrieval system** using RFID technology and an STM32 microcontroller. It replaces manual data entry with RFID-based automatic identification, making it faster and more accurate to retrieve computer component specifications.

### Why This Project?
- ⏱️ **Time-Saving**: Eliminates manual data entry
- ✅ **Accuracy**: Reduces human errors in specification recording
- 📊 **Centralized Database**: All specs stored in one place
- 🔄 **Scalable**: Can be expanded to track hundreds of components
- 💰 **Cost-Effective**: Affordable components with high ROI

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| **RFID-based Identification** | Each computer component tagged with unique RFID |
| **Real-time Processing** | Instant data processing by STM32 |
| **LCD Display** | 16x2 LCD shows component specifications |
| **WiFi Connectivity** | NodeMCU enables IoT capabilities |
| **Centralized Database** | All specs stored for easy access |
| **User-friendly Interface** | Simple scan and view operation |
| **Scalable Architecture** | Can handle multiple components |
| **Low Power Consumption** | Energy-efficient design |

---

## 🏗️ System Architecture

### Block Diagram



<p align="center">
  <img width="1157" height="697" alt="Block Diagram" src="https://github.com/user-attachments/assets/3d5ccb43-52b2-4512-a51e-3819ecb9604f" />
  <br>
  <em>Figure: Block Diagram </em>
</p>


### Working Principle

1. **SCAN** 🎫 → User brings RFID tag near EM-18 reader
2. **READ** 📡 → Reader detects tag and sends unique ID to STM32 via UART
3. **PROCESS** ⚙️ → STM32 matches ID with pre-stored database
4. **DISPLAY** 🖥️ → Component specifications shown on LCD
5. **UPLOAD** ☁️ → Data sent to cloud via NodeMCU for remote access

---

## 🔧 HARDWARE COMPONENTS

| Component | Specifications | Quantity | Purpose |
|-----------|---------------|----------|---------|
| **STM32F103C8T6** | ARM Cortex-M3, 72MHz, 64KB Flash | 1 | Main microcontroller |
| **EM-18 RFID Reader** | 125kHz, UART interface, 5-10cm range | 1 | Reads RFID tags |
| **RFID Tags** | 125kHz, EM4100 compatible | 5+ | Attached to components |
| **NodeMCU ESP8266** | WiFi, 4MB Flash, 80MHz | 1 | Cloud connectivity |
| **16x2 LCD with I2C** | 16x2 characters, I2C interface | 1 | Display specifications |
| **Jumper Wires** | Male-to-Female, Male-to-Male | 20+ | Connections |
| **Breadboard** | 400/800 points | 1 | Prototyping |
| **Power Supply** | 5V/3.3V regulated | 1 | Power source |

**Total Cost:** ₹1500-1800 ($18-22)

---


## 💻 SOFTWARE REQUIREMENTS

### Development Tools
| Tool | Version | Purpose |
|------|---------|---------|
| Arduino IDE | 1.8.13+ | Programming STM32 & NodeMCU |
| STM32CubeMX | Optional | Pin configuration |
| Fritzing | Latest | Circuit diagrams |
| MySQL/XAMPP | Latest | Database management |

## 🎯 APPLICATIONS

- 🏫 **Educational Institutions** - Track computer lab inventory
- 🏢 **IT Departments** - Manage company hardware assets
- 💻 **Computer Repair Shops** - Quick customer system info
- 🏭 **Manufacturing** - Track production line computers
- 🏥 **Hospitals** - Monitor medical computer systems
- 🏨 **Hotels** - Manage business center computers

## 🚀 FUTURE ENHANCEMENTS

- [ ] **Mobile App** - Android/iOS app for scanning
- [ ] **Web Dashboard** - Real-time inventory monitoring
- [ ] **Email Alerts** - Notifications for low stock/repairs
- [ ] **Barcode Backup** - Fallback scanning method
- [ ] **User Authentication** - Role-based access control
- [ ] **Analytics Dashboard** - Usage patterns and predictions
- [ ] **Multiple Readers** - Network of RFID scanners
- [ ] **Battery Powered** - Portable handheld device

---

## 📄 License

This project is licensed under the MIT License – see the [LICENSE](LICENSE) file for details.

---

## 📬 Contact

For any queries, please contact:  
atharvasawant3183@gmail.com 

https://www.linkedin.com/in/atharvavsawant/

## 📚 REFERENCES

1. Zhang, H., & Kang, W. (2013). "Design of the Data Acquisition System Based on STM32." *Physics Procedia*, 37, 2223-2228. [ScienceDirect](https://www.sciencedirect.com/science/article/pii/S1877050913001634)

2. Bottani, E., Volpi, A., Rizzi, A., Montanari, R., & Bertolini, M. (2013). "Radio Frequency Identification Reader." *ScienceDirect Topics*. [Link](https://www.sciencedirect.com/topics/engineering/rfid-reader)

3. Deng, B., Bo, Z., Jia, Y., Gao, Z., & Liu, Z. (2020). "Research on STM32 Development Board Based on ARM Cortex-M3." *2020 IEEE 2nd International Conference on Civil Aviation Safety and Information Technology (ICCASIT)*.

4. STMicroelectronics. (2020). "STM32F103C8T6 Datasheet - Medium-density performance line ARM Cortex-M3 32-bit MCU." [ST.com](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)

5. Arduino. (2024). "Arduino IDE Documentation." [Arduino.cc](https://www.arduino.cc/en/Guide)


