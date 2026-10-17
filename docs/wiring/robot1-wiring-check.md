# Robot1: nối và kiểm tra từng dây

Cập nhật: 2026-10-17. Áp dụng cho sơ đồ `docs/robot1/diagram`: ESP32-C3 SuperMini, OLED SSD1306, mic INMP441, ampli MAX98357A.
Firmware kiểm tra: `projects/esp32-wiring-check` (đã build thành công, **chưa chạy trên board thật**).

## 1. Vì sao nối dây xong thì mất Wi-Fi

Kết luận từ source trong `pocket-esp-ai-project-complete.zip` (`main/boards/pocket-wall-e-c3/`):

1. **Pocket phát Wi-Fi tên `Xiaozhi-XXXX`**, không phải `NUNU-ESP32` (tên đó là của firmware Wi-Fi test). Trang cấu hình: `http://192.168.4.1`. Sau khi đã lưu Wi-Fi nhà, Pocket không phát mạng này nữa. Đây là hành vi bình thường.
2. **Lỗi OLED làm board khởi động lại liên tục trước khi bật Wi-Fi.** Firmware khởi tạo OLED ở địa chỉ cố định `0x3C` bằng `ESP_ERROR_CHECK`. OLED không trả lời (dây SDA/SCL lỏng, thiếu nguồn, địa chỉ `0x3D`) thì chip abort, reset, lặp lại, nên không bao giờ tới bước phát Wi-Fi. Vì vậy Pocket không chạy được trên board trần, và không dùng Pocket để kiểm tra dây.
3. **Tiếp xúc dây.** Ghi chú trước đây (`docs/notes/pocket-esp-ai-bringup.md`): dây chỉ luồn qua lỗ, chưa hàn. Hàn header trước khi làm tiếp.
4. **Anten.** Dòng SuperMini nổi tiếng anten yếu: dây, pin hoặc kim loại chạy sát đầu anten (đầu đối diện cổng USB-C) làm mất Wi-Fi. Một số board chỉ ổn định khi giảm công suất phát; firmware kiểm tra có lệnh `t` để thử.
5. **Nguồn.** Sụt áp khi Wi-Fi phát hoặc ampli kêu sẽ gây reset `BROWNOUT`. Firmware kiểm tra in lý do reset để bắt lỗi này.

## 2. Quy tắc an toàn (áp dụng mọi bước)

- **Tháo pin và mạch sạc ra hoàn toàn** trong suốt quá trình này. Sơ đồ nối pin thẳng vào chân 5V: nếu bật công tắc pin trong lúc cắm USB, USB có thể nạp thẳng vào pin Li-ion không qua mạch sạc. Phần pin để làm sau, có kiểm tra riêng.
- **Rút USB trước khi cắm hoặc rút bất kỳ dây nào.**
- Mỗi module: **nối GND trước, rồi nguồn, sau cùng mới tới dây tín hiệu.** Không để module có dây tín hiệu nhưng chưa có nguồn.
- Đi dây sang hai bên board, **không vắt qua đầu anten**. Đầu anten nhô ra ngoài mép breadboard, không đặt trên pin hoặc kim loại.
- Breadboard: dùng một thanh nguồn cho **3V3**, thanh nguồn bên kia cho **5V**, đánh dấu bằng băng dính. Không bao giờ nối 5V vào thanh 3V3.

## 3. Nạp firmware kiểm tra (một lần duy nhất)

Board trần, chỉ cắm USB:

```bash
cd ~/Desktop/nnlab/nunu-esp32-lab/projects/esp32-wiring-check
~/.platformio/penv/bin/pio run -t upload
~/.platformio/penv/bin/pio device monitor
```

Không vào được chế độ nạp: giữ BOOT, nhấn rồi thả RESET, thả BOOT, chạy lại `upload`.

Mỗi lần cắm lại USB, mở lại monitor rồi gõ `r` để in lại báo cáo khởi động. Các lệnh khác:

| Phím | Tác dụng |
| --- | --- |
| `r` | In lại báo cáo khởi động (kiểm tra chân, quét I2C, Wi-Fi) |
| `b` | Phát tiếng bíp 1 kHz trong 0.6 giây qua ampli |
| `t` | Đổi công suất phát Wi-Fi giữa 19.5 dBm và 8.5 dBm |
| `w` | Quét lại Wi-Fi xung quanh (số mạng thấy và RSSI mạnh nhất) |

Dòng trạng thái in mỗi 2 giây:

```
[   42s] reset=POWERON boots=1 | AP clients=1 tx=19.5dBm | pins=OK | oled=0x3C OK(err=0) | mic=-55 dBFS | btn=up | heap=...
```

| Trường | Bình thường | Bất thường |
| --- | --- | --- |
| `[42s]` | Tăng đều | Quay về 0 → board vừa reset |
| `reset` | `POWERON` | `BROWNOUT` → sụt nguồn; `PANIC` → crash |
| `boots` | 1 | Lớn hơn 1 → đang reset lặp |
| `AP clients` | 1 khi điện thoại đã nối vào `NUNU-WIRECHECK` | 0 dù điện thoại báo đã kết nối |
| `pins` | `OK` | `CHECK` → gõ `r`, tìm dòng có `<-- CHECK` |
| `oled` | `0x3C OK(err=0)`, `err` đứng yên | `none`, `ERRORS`, hoặc `err` tăng dần |
| `mic` | Khoảng −60 đến −40 dBFS khi yên lặng, tăng rõ khi vỗ tay | `STUCK at ...` hoặc `too loud` |

Mạng `NUNU-WIRECHECK` không có mật khẩu, chỉ để kiểm tra. Tắt board khi không dùng.

## 4. Quy trình 5 bước cho MỖI sợi dây

Làm đúng vòng này cho từng dây trong bảng ở mục 5, không nối nhiều dây một lúc.

**A. Rút USB.**

**B. Nối đúng một dây** theo bảng.

**C. Đo bằng đồng hồ vạn năng (còn rút USB)**, chế độ thông mạch (bíp) hoặc Ω:
1. Que đặt lên **chân kim loại của ESP32** và **chân kim loại của module**, không đặt lên đầu dây. Phải bíp, điện trở gần như số đo khi chập hai que.
2. Giữ que, **lay nhẹ dây và chân**. Tiếng bíp phải liên tục; nếu ngắt quãng là tiếp xúc lỏng, sửa trước khi đi tiếp.
3. Đo chân module vừa nối với **hai chân ESP32 kế bên** (ví dụ vừa nối GPIO2 thì đo với GPIO1 và GPIO3). Phải **không** bíp.
4. Nếu dây vừa nối là nguồn: đo giữa 3V3 và GND, rồi 5V và GND. Phải **không** về gần 0 Ω. Bíp thoáng rồi tắt là do tụ, chấp nhận được.

**D. Cắm USB, mở monitor, gõ `r`.** So với cột "Kết quả mong đợi". Khi dây vừa nối là nguồn, đo điện áp (V DC, que đen ở GND) **ngay tại chân module**.

**E. Kiểm tra Wi-Fi bằng điện thoại**: thấy `NUNU-WIRECHECK`, kết nối được (`AP clients=1`), để yên 60 giây mà `[...s]` không quay về 0.

Ghi kết quả từng bước. Nếu bước nào hỏng: rút USB, tháo **chính dây vừa nối**, chạy lại để xác nhận lỗi biến mất, rồi xem mục 6.

## 5. Thứ tự nối và kết quả mong đợi

### Bước 0: mốc chuẩn

| # | Việc làm | Kết quả mong đợi |
| --- | --- | --- |
| 0.1 | Board trần, chỉ USB | `pins=OK`, mọi GPIO `open` (GPIO8 có thể `HIGH: pull-up present` vì LED trên board), `oled=none`, `mic=STUCK`. Wi-Fi OK. **Ghi lại** dòng `Wi-Fi receive check` (số mạng, RSSI mạnh nhất). |
| 0.2 | Hàn header, cắm board lên breadboard, chưa nối dây | Giống 0.1. RSSI mạnh nhất không giảm quá ~10 dB so với 0.1. |

Nếu 0.1 đã mất Wi-Fi hoặc điện thoại không kết nối được: gõ `t` chuyển sang 8.5 dBm rồi thử lại. Chạy được ở 8.5 dBm mà không chạy ở 19.5 dBm là lỗi của board/anten, không phải lỗi dây; báo lại để mình chỉnh Pocket build với công suất thấp hơn.

### Bước 1: thanh nguồn

| # | Dây | Kết quả mong đợi |
| --- | --- | --- |
| 1.1 | ESP32 `G` → thanh GND | Giống bước 0 |
| 1.2 | ESP32 `3.3` → thanh 3V3 | 3V3–GND đo được 3.2–3.4 V |

### Bước 2: OLED (VCC dùng 3V3)

| # | Dây | Kết quả mong đợi |
| --- | --- | --- |
| 2.1 | OLED `GND` → thanh GND | Giống bước 1 |
| 2.2 | OLED `VCC` → thanh 3V3 | Đo tại chân VCC–GND của OLED: 3.2–3.4 V |
| 2.3 | OLED `SCL` → GPIO20 | `GPIO20 OLED SCL ... HIGH: pull-up present` |
| 2.4 | OLED `SDA` → GPIO21 | `GPIO21 ... HIGH: pull-up present`, `I2C scan: 0x3C`, màn hình nhấp nháy ô bàn cờ, `oled=0x3C OK(err=0)` |

Sau 2.4, lay nhẹ từng dây OLED khi đang chạy: `err` phải đứng yên, màn hình không đứng hình. Mốc này quan trọng nhất cho Pocket.

### Bước 3: mic INMP441 (VDD dùng 3V3)

| # | Dây | Kết quả mong đợi |
| --- | --- | --- |
| 3.1 | Mic `GND` → thanh GND | Giống bước 2 |
| 3.2 | Mic `L/R` → thanh GND | Giống bước 2 |
| 3.3 | Mic `VDD` → thanh 3V3 | Đo tại VDD–GND của mic: 3.2–3.4 V |
| 3.4 | Mic `SCK` → GPIO2 | `GPIO2` vẫn `open`, `pins=OK` |
| 3.5 | Mic `WS` → GPIO1 | `GPIO1` vẫn `open`, `pins=OK` |
| 3.6 | Mic `SD` → GPIO8 | `mic` hết `STUCK`; yên lặng khoảng −60 đến −40 dBFS, nói hoặc vỗ tay sát mic tăng thêm ít nhất 15 dB |

Trước 3.6, `mic=STUCK` là bình thường. Sau 3.6 vẫn `STUCK` thì kiểm tra lại SD, SCK, WS, VDD.

### Bước 4: ampli MAX98357A (VIN dùng 5V)

| # | Dây | Kết quả mong đợi |
| --- | --- | --- |
| 4.1 | Ampli `GND` → thanh GND | Giống bước 3 |
| 4.2 | ESP32 `5V` → thanh 5V, rồi ampli `Vin` → thanh 5V | Đo tại Vin–GND của ampli: khoảng 4.6–5.1 V. Thanh 3V3 vẫn 3.2–3.4 V |
| 4.3 | Ampli `BCLK` → GPIO2 (cùng hàng breadboard với SCK của mic) | `pins=OK`, mic vẫn có tín hiệu |
| 4.4 | Ampli `LRC` → GPIO1 (cùng hàng với WS của mic) | `pins=OK`, mic vẫn có tín hiệu |
| 4.5 | Ampli `DIN` → GPIO3 | `pins=OK` |
| 4.6 | Rút USB, nối loa vào hai cọc `+` và `−` của ampli. Không nối cọc nào xuống GND | Gõ `b`: nghe tiếng bíp. Trong và sau tiếng bíp: không reset, `reset` vẫn `POWERON`, Wi-Fi còn |

Để trống `GAIN` và `SD` của ampli như sơ đồ.

### Bước 5 (tùy chọn): nút chức năng

Firmware Pocket có dùng nút ở GPIO10 (một đầu GPIO10, đầu kia GND) để vào lại chế độ cấu hình Wi-Fi. Sơ đồ không vẽ nút này.

| # | Dây | Kết quả mong đợi |
| --- | --- | --- |
| 5.1 | Nút: GPIO10 ↔ GND | `btn=up`, khi nhấn giữ thì `btn=DOWN` |

### Bước 6: kiểm tra tổng

Mọi dây đã nối, chạy 5 phút: `boots=1`, `[...s]` không quay về 0, `oled err` đứng yên, mic còn tín hiệu, gõ `b` vài lần vẫn ổn, điện thoại vẫn giữ kết nối. RSSI (`w`) không thấp hơn mốc 0.2 quá ~10 dB.

## 6. Bảng xử lý lỗi

| Triệu chứng | Nguyên nhân thường gặp | Cách xử lý |
| --- | --- | --- |
| `LOW: tied to GND` ở chân tín hiệu | Dây chạm GND, cầu thiếc, hoặc module đó chưa có nguồn (OLED thiếu VCC thì SDA/SCL bị kéo xuống) | Kiểm tra VCC/GND của module trước, sau đó tìm chỗ chạm |
| `HIGH: tied to 3V3/5V?` ở GPIO1/2/3/10 hoặc chân `unused` | Dây cắm nhầm hàng trên breadboard, chạm vào thanh nguồn | Đối chiếu số chân in trên board, không dựa vào màu dây |
| `BRIDGE: GPIOx and GPIOy` | Hai dây tín hiệu chạm nhau: cầu thiếc giữa hai chân kề, hoặc cắm chung hàng | Tách ra, đo lại bước C.3 |
| `I2C scan: nothing` | SDA/SCL đảo nhau, thiếu VCC/GND, hoặc dây lỏng | Đổi SDA/SCL, đo lại nguồn tại chân OLED |
| `I2C scan: 0x3D` | Jumper địa chỉ mặt sau OLED đặt ở `0x7A` | Chuyển điện trở/jumper sang `0x78` (= 0x3C). Pocket bắt buộc dùng 0x3C |
| `oled err` tăng dần | Tiếp xúc SDA/SCL chập chờn | Hàn lại hoặc thay dây; lay thử để tìm dây lỗi |
| `mic=STUCK` sau khi nối đủ | Thiếu VDD/GND, SCK/WS/SD lỏng hoặc sai chân | Đo lại từng dây của mic theo bước C |
| `mic ... too loud` dù yên lặng | SD lơ lửng (không tiếp xúc) | Kiểm tra dây SD → GPIO8 |
| `reset=BROWNOUT` hoặc `[...s]` về 0 khi bíp hoặc khi Wi-Fi phát | Nguồn yếu: cáp USB dài/kém, hub USB, chập nhẹ | Dùng cáp ngắn tốt, cắm thẳng vào máy; đo 5V/3V3 lúc bíp |
| Wi-Fi mất sau khi thêm một dây, nhưng `pins=OK` và không reset | Dây chạy qua vùng anten | Đi lại dây xa đầu anten; thử `t` (8.5 dBm) để so sánh |

## 7. Nạp lại Pocket sau khi mọi bước đều qua

Rút USB, chỉ giữ dây như sơ đồ (không pin). Cắm USB, đóng monitor, rồi:

```bash
cd ~/Desktop/nnlab/nunu-esp32-lab
esptool --chip esp32c3 --port /dev/ttyACM0 erase_flash
esptool --chip esp32c3 --port /dev/ttyACM0 --baud 460800 write_flash 0x0 docs/robot1/pocket-esp-ai.bin
```

`erase_flash` xóa sạch dữ liệu firmware kiểm tra để Pocket khởi động sạch. File là ảnh gộp, luôn ghi ở `0x0`. Thay `/dev/ttyACM0` bằng cổng thật (`python3 -m serial.tools.list_ports`).

Sau khi nạp: OLED hiện mắt, điện thoại tìm mạng **`Xiaozhi-XXXX`**, kết nối rồi mở `http://192.168.4.1` để nhập Wi-Fi nhà. Sau đó mạng `Xiaozhi-XXXX` biến mất là bình thường. Muốn vào lại cấu hình: nhấn giữ nút GPIO10.

Xem log Pocket: `python3 -m serial.tools.miniterm --raw /dev/ttyACM0 115200`. Gặp `ESP_ERROR_CHECK failed` ngay sau dòng liên quan đến SSD1306/I2C thì quay lại bước 2.

## 8. Pin và mạch sạc: để sau

Sơ đồ đưa thẳng cực + của pin (qua công tắc) vào chân 5V. Trước khi làm phần này cần biết: model mạch sạc, pin có mạch bảo vệ không, board SuperMini có diode giữa USB và chân 5V không. Không bật công tắc pin khi đang cắm USB cho tới khi đã kiểm tra xong.
