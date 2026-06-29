import serial
import csv
import time

# TÊN CỔNG COM: Thay đổi cho đúng với cổng máy tính bạn đang nhận ESP32 (VD: 'COM12', 'COM3')
COM_PORT = 'COM12' 
BAUD_RATE = 115200
FILE_NAME = 'du_lieu_phan_tich.csv'

try:
    # Mở kết nối Serial
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print(f"Đã kết nối thành công với {COM_PORT}.")
    print(f"Đang ghi dữ liệu vào file: {FILE_NAME} (Nhấn Ctrl+C để dừng)...")
    
    # Mở file CSV để ghi thêm (append mode)
    with open(FILE_NAME, mode='a', newline='', encoding='utf-8') as file:
        writer = csv.writer(file)
        
        # Ghi Header nếu file trống (Bạn có thể bỏ qua nếu file đã có Header)
        # writer.writerow(["Date", "Time", "Temp", "Humidity", "Pressure", "GasRes", "CO2", "PM1.0", "PM2.5", "PM10"])
        
        while True:
            if ser.in_waiting > 0:
                # Đọc luồng dữ liệu từ ESP32 gửi lên
                line = ser.readline().decode('utf-8').strip()
                
                if line:
                    # Tách chuỗi bằng dấu phẩy và ghi vào file CSV
                    data_row = line.split(',')
                    writer.writerow(data_row)
                    file.flush() # Ép lưu ngay lập tức vào ổ cứng
                    print(f"Đã lưu: {line}")
                    
except serial.SerialException:
    print(f"LỖI: Không thể mở cổng {COM_PORT}. Hãy kiểm tra xem cổng có đúng không hoặc VS Code đang chiếm quyền sử dụng?")
except KeyboardInterrupt:
    print("\nĐã dừng thu thập dữ liệu.")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()