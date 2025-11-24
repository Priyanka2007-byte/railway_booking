# 🚆 Railway Ticket Booking System (C Language)

A powerful and feature-rich **Railway Ticket Booking System** developed in C.  
This project includes advanced capabilities such as **QR code ticket generation** and **fraud-prevention duplicate booking checks**, making it highly professional and industry-level.

---

## 📚 Table of Contents
1. About the Project  
2. Features  
3. Unique Features  
4. Flowchart  
5. Output Screenshots  
6. Project Structure  
7. How to Compile & Run  
8. Sample Output  
9. Future Enhancements  
10. License  

---

## 📝 1. About the Project
This project is designed to replicate real-world railway ticketing logic.  
It showcases:
- Structured programming  
- File handling  
- Linked lists  
- Data validation  
- QR ticket generation  
- Duplicate booking prevention  

All booking data is stored permanently in `bookings.dat`, and the system automatically restores bookings on launch.

---

## ⭐ 2. Features

- 📋 View available trains  
- 🎫 Book tickets  
- 📁 Save tickets using file handling  
- 🔍 Search bookings  
- ❌ Cancel bookings  
- 🔄 Auto-generate booking IDs  
- 🧮 Seat availability check  
- 🗃️ Auto-recovery system (loads previous bookings automatically)

---

## 🧩 3. Unique Features (Advanced)

### 🔐 **Fraud Prevention: Duplicate Booking Check**
The system automatically detects if a passenger tries to book:
- Same name  
- Same age  
- Same train  
- Same travel class  

If detected → **Booking is blocked**  
This feature prevents fraudulent repeated bookings.

---

### 🔳 **QR Code Ticket Generator**
Every successful booking generates:

- A **ticket file** (`booking_<id>.txt`)  
- A corresponding **QR code**:  
  - Real QR (`booking_<id>_qr.pbm`) if `libqrencode` is installed  
  - ASCII-art QR placeholder (`booking_<id>_qr.txt`) if not

This QR contains booking details:
BookingID:<id>;Name:<name>;Age:<age>;Train:<train>;Class:<class>

---

## 🧠 4. Flowchart

> Upload your flowchart to:  
> `![alt text](flowchartsflowchart.png.png)`

Example:

![Flowchart](flowcharts/flowchart.png)

---

## 📸 5. Output Screenshots

> Upload your screenshots to:  
> `images`

### **Main Menu**
![Menu Screen](![alt text](imagesmenu.png.png))

### **Booking Output**
![Booking Output](![alt text](imagesbooking.png.png))

---

## 🗂️ 6. Project Structure

Railway-Booking/
├── railway_booking_qr.c → Full source code
├── README.md → Documentation
├── bookings.dat → Auto-created booking storage
├── images/ → Output screenshots
├── flowcharts/ → System flowchart

---

## 🛠️ 7. How to Compile & Run

### ✔ Without QR library (ASCII QR mode)
gcc railway_booking_qr.c -o railway_booking
./railway_booking

---

## 🧪 8. Sample Output

================ Railway Ticket Booker ================

List Trains

Book Ticket

View All Bookings

Search Booking by ID

Cancel Booking

Exit
Enter choice:

---

## 🚀 9. Future Enhancements

- 🪟 GUI interface  
- 📄 Export ticket to PDF  
- 😀 Multi-passenger booking  
- 🗺️ Seat map visualization  
- 🔐 User login system  

---

## 📜 10. License
This project is licensed under the **MIT License**.

---

# 👨‍💻 Author
Developed by **PRIYANKA BARIK / Priyanka2007-byte**  
Made with ❤️ for learning C and mastering GitHub.

