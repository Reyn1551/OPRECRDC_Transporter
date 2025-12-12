# OPRECRDC Transporter Robot

**Dokumentasi Lengkap Proyek Robot Dua Mikrokontroler (ESP32 & Arduino Nano)**

Proyek ini adalah sistem robot transporter yang dikendalikan melalui Web Interface (via WiFi). Sistem ini menggunakan arsitektur *dual-microcontroller* untuk memisahkan beban kerja:
1.  **ESP32**: Bertindak sebagai "Otak" dan Antarmuka. Menangani WiFi, Web Server, dan menerima input dari pengguna.
2.  **Arduino Nano**: Bertindak sebagai "Otot" atau Driver. Menerima perintah dari ESP32 dan menggerakkan motor serta servo secara langsung.

---

## 🏗️ Arsitektur Sistem

Modul komunikasi yang digunakan adalah **Serial UART**. Berikut alur datanya:

`[User / HP]` --(WiFi)--> `[ESP32]` --(Serial TX-RX)--> `[Arduino Nano]` --(GPIO/PWM)--> `[Motor & Servo]`

*   **ESP32** membuat Hotspot WiFi (`Robot_Controller`).
*   Pengguna mengakses IP `192.168.4.1` untuk membuka Web Controller.
*   Saat tombol ditekan, ESP32 mengirim karakter perintah (misal: 'F' untuk maju) ke Arduino Nano.
*   Arduino Nano membaca karakter tersebut dan mengeksekusi gerakan motor atau servo.

---

## 🔌 Persiapan Hardware & Wiring

Berikut adalah daftar komponen dan sambungan kabel yang dikonfigurasi dalam kode saat ini.

### 1. Komponen Utama
*   1x ESP32 Development Board (DOIT DEVKIT V1 atau sejenisnya)
*   1x Arduino Nano
*   1x Modul Driver Motor L298N (atau sejenis)
*   3x Servo Motor (Untuk Gripper Kiri, Kanan, dan Lift)
*   Sumber Daya (Baterai Li-Ion/Li-Po disarankan untuk Motor dan Servo)

### 2. Wiring ESP32
| Pin ESP32 | Terhubung Ke | Fungsi |
| :--- | :--- | :--- |
| **GND** | GND (Common) | Ground (Wajib disatukan dengan Nano & Baterai) |
| **VIN/5V** | 5V Power | Input Daya |
| **GPIO 16 (RX2)** | TX Arduino Nano | Menerima data dari Nano (jika ada) |
| **GPIO 17 (TX2)** | RX Arduino Nano | Mengirim perintah ke Nano |

> **⚠️ PENTING:** Pastikan **GND** (Ground) semua komponen (ESP32, Nano, Driver Motor, Baterai) terhubung menjadi satu (Common Ground). Jika tidak, komunikasi Serial tidak akan bekerja.

### 3. Wiring Arduino Nano
#### Koneksi ke Motor Driver (L298N)
| Pin Nano | Pin Driver Motor | Fungsi |
| :--- | :--- | :--- |
| **D2** | ENA | PWM Speed Motor Kiri |
| **D3** | IN1 | Arah Motor Kiri A |
| **D4** | IN2 | Arah Motor Kiri B |
| **D5** | IN3 | Arah Motor Kanan A |
| **D6** | IN4 | Arah Motor Kanan B |
| **D7** | ENB | PWM Speed Motor Kanan |

#### Koneksi ke Servo
| Pin Nano | Servo | Fungsi |
| :--- | :--- | :--- |
| **D8** | Servo Kanan | Mencapit (Gripper Rights) |
| **D9** | Servo Kiri | Mencapit (Gripper Left) |
| **D10** | Servo Lift | Mengangkat (Lift Up/Down) |

#### Koneksi Komunikasi (Serial)
| Pin Nano | Terhubung Ke | Catatan |
| :--- | :--- | :--- |
| **RX (D0)** | Pin 17 (TX) ESP32 | Menerima perintah |
| **TX (D1)** | Pin 16 (RX) ESP32 | Mengirim status (opsional) |

---

## 🚀 Panduan Instalasi & Upload

### Langkah 1: Upload ke ESP32
1.  Buka file `Esp32.ino`.
2.  Pilih Board **DOIT ESP32 DEVKIT V1** di Arduino IDE.
3.  Pastikan library `WiFi` dan `WebServer` sudah terinstall (bawaan paket ESP32).
4.  Hubungkan ESP32 ke PC, lalu **Upload**.

### Langkah 2: Upload ke Arduino Nano
1.  Buka file `Nano.ino`.
2.  Pilih Board **Arduino Nano** di Arduino IDE.
3.  Pastikan library `Servo` sudah terinstall.
4.  **⚠️ PENTING:** Sebelum upload, **CABUT KABEL** yang terhubung ke **Pin RX (0)** dan **TX (1)** pada Nano.
    *   *Alasannya: Pin 0 dan 1 digunakan oleh USB untuk upload program. Jika ada kabel tertancap, upload akan gagal.*
5.  Hubungkan Nano ke PC, lalu **Upload**.
6.  Setelah sukses alias "Done uploading", **PASANG KEMBALI** kabel RX/TX.

---

## 🎮 Cara Penggunaan

1.  Nyalakan Robot (Pastikan baterai terpasang).
2.  Di HP/Laptop, cari WiFi bernama:
    *   **SSID**: `Robot_Controller`
    *   **Password**: `12345678`
3.  Buka Browser (Chrome/Safari), ketik alamat: **`192.168.4.1`**
4.  Akan muncul tampilan **Controller**.
    *   **Landscape Mode**: Putar HP ke posisi landscape agar tombol terlihat rapi.
    *   Tampilan full screen disarankan (klik ikon pojok kanan atas).
5.  Gunakan tombol arah untuk menggerakkan robot dan tombol fungsi untuk menggerakkan Gripper/Lift.

---

## 🧠 Penjelasan Teknis & "How It Works"

### Protokol Komunikasi
Sistem menggunakan protokol satu karakter sederhana (`char`) untuk efisiensi.
Berikut kamus perintah yang dikirim dari ESP32 ke Nano:

| Karakter | Fungsi | Penjelasan |
| :---: | :--- | :--- |
| **'F'** | Maju (Forward) | Kedua motor maju |
| **'B'** | Mundur (Backward) | Kedua motor mundur |
| **'L'** | Kiri (Left) | Motor kiri mundur, kanan maju (Pivot) |
| **'R'** | Kanan (Right) | Motor kiri maju, kanan mundur (Pivot) |
| **'P'** | Stop (Pause) | Semua motor berhenti |
| **'T'** | Lift UP (Triangle) | Servo Lift naik (sudut 180) |
| **'X'** | Lift DOWN (Cross) | Servo Lift turun (sudut 95) |
| **'C'** | Gripper OPEN (Circle)| Membuka capit |
| **'S'** | Gripper CLOSE (Square)| Menutup capit |
| **'A'** | Kalibrasi (Start) | Set posisi servo ke default/nol |
| **'V'** | Speed | Prefix untuk set kecepatan (diikuti angka) |

### Logika Program
1.  **ESP32 (`Esp32.ino`)**:
    *   Menyajikan halaman HTML yang tersimpan dalam variabel `index_html`.
    *   Menggunakan AJAX (`XMLHttpRequest`) di JavaScript untuk mengirim perintah tanpa me-refresh halaman.
    *   Fungsi `handleCommand()` menangkap request `/cmd?val=X` dan meneruskannya ke Serial2 (`Serial2.print`).

2.  **Arduino Nano (`Nano.ino`)**:
    *   Loop utama (`void loop`) memantau `Serial.available()`.
    *   Jika ada data masuk, data dibaca dan diproses di `processBluetoothCommand()`.
    *   Berdasarkan karakter yang diterima, fungsi `motorCtrl` atau `gripperCtrl` dipanggil untuk mengubah state pin digital/PWM.

---

## 🛠️ Panduan Modifikasi (Customization)

Bagian ini menjelaskan cara mengubah kode jika Anda ingin mengganti pin atau mengubah logika.

### 1. Mengubah Pin Motor & Servo (Pada Nano)
Jika Anda menggunakan kaki/pin yang berbeda di Arduino Nano, ubah bagian ini di `Nano.ino`:

```cpp
struct MotorConfig {
    // Ubah angka pin di sini sesuai wiring Anda
    const uint8_t ENA = 2; // Contoh: ganti jadi 10
    const uint8_t IN1 = 3;
    ...
};

struct GripperConfig {
    // Ubah pin servo di sini
    const uint8_t GRIPPER_RIGHT_PIN = 8;
    ...
};
```

### 2. Kalibrasi Sudut Servo (Pada Nano)
Jika servo Anda bergerak terlalu jauh atau kurang jauh (mentok/bunyi berisik), ubah nilai sudutnya di `Nano.ino`:

```cpp
struct GripperConfig {
    // Sesuaikan sudut (range 0 - 180)
    const uint8_t GRIPPER_LEFT_OPEN = 20;   // Ubah jika kurang terbuka
    const uint8_t GRIPPER_LEFT_CLOSE = 51;  // Ubah jika jepitan terlalu keras/longgar
    const uint8_t LIFT_UP = 180;            
    const uint8_t LIFT_DOWN = 95;           
    ...
};
```

### 3. Mengubah Tampilan Web Interface (Pada ESP32)
Tampilan web ada di dalam variabel `index_html` di file `Esp32.ino`. Code ini adalah HTML/CSS murni.
*   **Mengganti Warna**: Cari bagian style CSS, misal `.btn-move` untuk tombol gerak.
*   **Mengubah Posisi Tombol**: Edit bagian Grid CSS (`grid-column`, `grid-row`).

### 4. Mengubah Kecepatan Default (Pada Nano)
Kecepatan motor diatur via PWM (0-255). Defaultnya ada di struktur motor:
```cpp
const uint8_t DEFAULT_SPEED = 140; // Ganti nilai ini (0-255)
```

---

## ❓ Troubleshooting (Masalah Umum)

**Q: Robot tidak bergerak saat tombol ditekan di Web.**
*   Cek apakah ESP32 dan Nano sudah terhubung *Common Ground* (GND disatukan).
*   Pastikan kabel RX ESP32 ke TX Nano, dan TX ESP32 ke RX Nano (Silang).
*   Cek apakah Driver Motor saklar power-nya sudah ON.

**Q: Web interface tidak bisa dibuka.**
*   Pastikan Anda sudah connect ke WiFi `Robot_Controller`.
*   Matikan paket data (Mobile Data) di HP, karena kadang HP menolak koneksi WiFi tanpa internet.

**Q: Tidak bisa upload program ke Nano.**
*   Cabut dulu kabel yang menancap di Pin 0 (RX) dan Pin 1 (TX) saat proses upload. Pasang lagi setelah selesai.
