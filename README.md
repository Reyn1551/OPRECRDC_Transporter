# Proyek Mobil RC dengan Kontrol Web Berbasis ESP32

Proyek ini adalah implementasi mobil robot (RC Car) yang dapat dikendalikan sepenuhnya melalui antarmuka web. ESP32 bertindak sebagai otak utama, menjalankan server web yang dapat diakses oleh perangkat apa pun (seperti smartphone, laptop, atau tablet) yang terhubung ke jaringan WiFi yang sama.

Ini adalah contoh bagus dari aplikasi Internet of Things (IoT) dalam robotika, di mana kontrol fisik dilakukan dari jarak jauh melalui jaringan.

## Daftar Isi
1. [Fitur Utama](#fitur-utama)
2. [Cara Kerja](#cara-kerja)
3. [Komponen & Perangkat Keras](#komponen--perangkat-keras)
4. [Pustaka (Library) yang Digunakan](#pustaka-library-yang-digunakan)
5. [Konfigurasi & Instalasi](#konfigurasi--instalasi)
6. [Struktur Kode](#struktur-kode)

---

## Fitur Utama
- **Kontrol Nirkabel via WiFi**: Tidak perlu remote kontrol fisik, cukup gunakan browser web.
- **Antarmuka Web Modern**: Tampilan kontrol yang responsif dan intuitif, berfungsi baik di perangkat mobile maupun desktop.
- **Kontrol Gerak Penuh**: Maju, mundur, belok kiri, belok kanan, dan berhenti.
- **Kontrol Kecepatan Dinamis**: Kecepatan motor dapat diatur secara *real-time* menggunakan slider (0-255 PWM).
- **Kontrol Gripper**: Fungsi untuk membuka dan menutup capit (gripper).
- **Kontrol Pengangkat**: Fungsi untuk menaikkan dan menurunkan mekanisme gripper.
- **Umpan Balik Status**: Antarmuka web menampilkan status terkini dari robot (kecepatan, status gripper, dan status pengangkat).

## Cara Kerja
1.  **Inisialisasi**: Saat ESP32 dinyalakan, ia akan mencoba terhubung ke jaringan WiFi yang telah ditentukan dalam kode (`ssid` dan `password`).
2.  **Menjalankan Web Server**: Setelah berhasil terhubung, ESP32 akan memulai server web pada alamat IP lokalnya (misalnya, `192.168.1.10`). Alamat IP ini akan ditampilkan di Serial Monitor Arduino IDE.
3.  **Akses Antarmuka**: Pengguna membuka browser di smartphone atau laptop, lalu mengakses alamat IP ESP32. Server akan mengirimkan halaman web (HTML, CSS, JavaScript) sebagai antarmuka kontrol.
4.  **Pengiriman Perintah**: Setiap kali pengguna menekan tombol pada antarmuka web (misalnya, tombol 'Maju' atau menggeser slider kecepatan), JavaScript akan mengirimkan permintaan HTTP ke alamat spesifik di server ESP32 (misalnya, `http://<IP_ESP32>/forward`).
5.  **Eksekusi Perintah**: ESP32 menerima permintaan tersebut, memprosesnya, dan langsung menggerakkan motor atau servo sesuai perintah yang diterima.
6.  **Respon**: ESP32 mengirimkan kembali respons dalam format JSON untuk mengonfirmasi bahwa perintah telah diterima (misalnya, `{"status":"forward"}`).

## Komponen & Perangkat Keras
Proyek ini dirancang untuk bekerja dengan komponen-komponen berikut:

- **Mikrokontroler**: ESP32 Dev Kit.
- **Driver Motor**: L298N atau driver sejenis yang dapat mengontrol dua motor DC.
- **Robot Chassis**: Rangka mobil dengan 2 atau 4 motor DC.
- **Gripper**: Mekanisme capit yang digerakkan oleh motor servo atau motor DC kecil.
- **Mekanisme Pengangkat**: Motor servo atau motor DC untuk menaikkan/menurunkan gripper.

### Konfigurasi Pin
Pastikan koneksi perangkat keras Anda sesuai dengan definisi pin dalam kode:

```cpp
// /home/reynboo/Arduino/Req_Galih/Req_Galih.ino

// Konfigurasi Motor
const uint8_t A_FORWARD = 25;   // Motor Kiri Maju
const uint8_t A_BACKWARD = 26;  // Motor Kiri Mundur
const uint8_t B_FORWARD = 33;   // Motor Kanan Maju
const uint8_t B_BACKWARD = 32;  // Motor Kanan Mundur
const uint8_t ENA = 27;         // PWM (Kecepatan) Motor Kiri
const uint8_t ENB = 14;         // PWM (Kecepatan) Motor Kanan

// Konfigurasi Gripper
const uint8_t GRIPPER_PIN = 12; // Pin untuk servo/motor gripper
const uint8_t LIFT_PIN = 13;    // Pin untuk motor pengangkat
```

## Pustaka (Library) yang Digunakan
Proyek ini bergantung pada beberapa pustaka Arduino. Pastikan Anda telah menginstalnya melalui **Library Manager** di Arduino IDE.

1.  **`WiFi.h`**
    - **Fungsi**: Pustaka standar ESP32 untuk mengelola koneksi WiFi.
    - **Instalasi**: Sudah terintegrasi dengan board ESP32 di Arduino IDE, tidak perlu instalasi manual.

2.  **`WebServer.h`**
    - **Fungsi**: Pustaka standar ESP32 untuk membuat server web sederhana yang menangani permintaan HTTP.
    - **Instalasi**: Sudah terintegrasi dengan board ESP32, tidak perlu instalasi manual.

3.  **`ArduinoJson`** (oleh Benoit Blanchon)
    - **Fungsi**: Digunakan untuk membuat dan mengirim respons JSON dari server ESP32 ke antarmuka web. Ini adalah cara standar untuk berkomunikasi dalam aplikasi web modern.
    - **Instalasi**: Buka Arduino IDE, pergi ke `Sketch > Include Library > Manage Libraries...`, cari `ArduinoJson`, dan instal. Versi yang digunakan dalam proyek ini adalah `7.3.0`.

## Konfigurasi & Instalasi
1.  **Pasang Library**: Pastikan semua pustaka di atas sudah terpasang.
2.  **Konfigurasi WiFi**: Buka file `.ino` dan ubah nilai `ssid` dan `password` agar sesuai dengan jaringan WiFi Anda.
    ```cpp
    // /home/reynboo/Arduino/Req_Galih/Req_Galih.ino
    const char* ssid = "NAMA_WIFI_ANDA";
    const char* password = "PASSWORD_WIFI_ANDA";
    ```
3.  **Upload Kode**: Hubungkan board ESP32 Anda ke komputer, pilih board dan port yang benar di Arduino IDE, lalu unggah kodenya.
4.  **Cari Alamat IP**: Buka **Serial Monitor** (baud rate 115200). Setelah ESP32 terhubung ke WiFi, alamat IP-nya akan ditampilkan.
5.  **Mulai Mengontrol**: Buka browser web di perangkat yang terhubung ke jaringan yang sama, lalu masukkan alamat IP tersebut. Halaman kontrol akan muncul.

## Struktur Kode
- **`Req_Galih.ino`**: File utama yang berisi semua logika.
    - **Struct `MotorConfig` & `GripperConfig`**: Mengelompokkan semua pin dan nilai konfigurasi untuk kemudahan modifikasi.
    - **`initializePins()`**: Fungsi untuk mengatur mode semua pin (OUTPUT).
    - **Class `MotorController`**: Mengabstraksi logika kontrol motor (maju, mundur, belok, berhenti, atur kecepatan).
    - **Class `GripperController`**: Mengabstraksi logika kontrol gripper dan pengangkat.
    - **`getOptimizedHTML()`**: Fungsi yang menghasilkan seluruh kode HTML, CSS, dan JavaScript untuk antarmuka web. Ini membuat proyek mandiri tanpa perlu menyimpan file HTML terpisah.
    - **`setupAPIEndpoints()`**: Mendefinisikan semua "rute" atau "endpoint" untuk server web (misalnya, apa yang harus dilakukan ketika permintaan ke `/forward` diterima).
    - **`setup()`**: Fungsi inisialisasi utama: menghubungkan WiFi, menginisialisasi pin, dan memulai server web.
    - **`loop()`**: Fungsi utama yang terus berjalan untuk menangani permintaan klien yang masuk ke server.