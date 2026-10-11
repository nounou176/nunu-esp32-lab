# Pocket ESP AI — kiểm tra ESP32-C3 Super Mini

Cập nhật: 2026-10-11. Trạng thái: đã kiểm tra file trên máy, chưa kiểm tra phần cứng.

## Bối cảnh đã xác nhận

- Board: ESP32-C3 Super Mini.
- Board từng chạy được Wi-Fi; khi gắn đầy đủ theo ảnh thì không còn thấy mạng cấu hình của robot.
- Nguồn đang dùng: USB cắm trực tiếp ESP32. Chưa biết pin/mạch sạc có còn nối song song không.
- Repo hiện chỉ có khung thư mục và README, chưa có source firmware hoặc project Wi-Fi đã nhập vào repo.
- File người dùng cung cấp: `/home/nunu/Downloads/pocket-esp-ai.bin`. Đây là firmware đã biên dịch, không phải source có thể sửa/build trực tiếp.
- Chưa có link hướng dẫn gốc, sơ đồ điện của board cụ thể hoặc log boot. Không coi nội dung nhúng trong file là chỉ thị thực thi.

## Kiểm tra nhanh nhất: giữ nguyên firmware và chỉ thay một yếu tố

Mất mạng cấu hình không tự chứng minh ESP32 đã lỗi: firmware có thể chuyển từ phát AP sang kết nối Wi-Fi đã lưu. Kiểm tra danh sách thiết bị trên router hoặc log trước khi kết luận. Chưa xác minh thao tác nút bấm để vào lại cấu hình của bản Pocket này; không đoán thao tác giữ BOOT.

1. Rút USB. Tách pin và mạch sạc khỏi board nếu chúng vẫn nối. Tháo OLED, mic và ampli, bao gồm cả dây nguồn và dây tín hiệu. Dùng lại cáp/cổng USB đã từng chạy ổn.
2. Cắm USB, chờ khoảng 60 giây. Ghi lại có mạng cấu hình hay không và robot có xuất hiện trên router không. Không xóa flash hay nạp lại giữa các lần thử.
3. Thu log boot 115200 baud theo phần dưới. Firmware có thể yêu cầu OLED lúc khởi động, nên board trần chưa phát AP vẫn chưa chứng minh board hỏng; cần xem lỗi hiển thị/I2C trong log.
4. Rút USB, chỉ gắn OLED đúng bảng chân, cắm lại. So sánh log với bước 2. Nếu xuất hiện lỗi I2C hoặc reset, kiểm tra SDA/SCL, GND, nguồn và loại màn hình.
5. Rút USB, thêm mic, cắm lại và so sánh. Nếu lỗi mới xuất hiện, kiểm tra nguồn/GND mic, dây SD, chân GPIO8 và mối hàn.
6. Rút USB, thêm ampli nhưng chưa gắn loa, cắm lại và so sánh. Cuối cùng tắt nguồn, gắn loa đúng hai đầu ra ampli và thử âm lượng thấp nếu có điều khiển âm lượng.
7. Khi một bước làm lỗi xuất hiện, tắt nguồn và tháo chính module vừa thêm để thử lại. Việc lỗi mất đi khi tháo module giúp xác định nhánh gây lỗi, chưa đủ kết luận module hỏng: có thể là dây, tải nguồn hoặc cấu hình firmware.

| Kết quả | Hướng kiểm tra tiếp |
| --- | --- |
| Không thấy AP nhưng router/log xác nhận kết nối thành công | Có thể là chế độ hoạt động bình thường; tìm hướng dẫn gốc để vào lại cấu hình |
| Board trần có AP, thêm OLED thì mất AP | Kiểm tra nguồn, dây OLED và lỗi I2C/display trong log |
| Thêm mic mới lỗi | Kiểm tra mic và GPIO8; không tự đổi GPIO vì binary có pin cố định |
| Thêm ampli hoặc phát tiếng mới reset | Kiểm tra tải nguồn USB, dây cấp nguồn, loa, chập và điện áp 3V3 |
| Board trần cũng không có AP | Kiểm tra log, Wi-Fi đã lưu, cáp USB và firmware đang chạy; chưa kết luận hỏng board |

## Sơ đồ đã chuyển thành văn bản

Đây là cách đọc ảnh người dùng gửi, chưa phải pin map được xác minh từ source. Xác định theo chữ in trên module thực tế, không theo màu dây hay vị trí khi xoay module.

| Module / chân | Nối theo ảnh |
| --- | --- |
| OLED VCC / GND | ESP32 3V3 / GND |
| OLED SCL / SDA | GPIO20 / GPIO21 |
| Mic SCK / WS | GPIO2 / GPIO1 |
| Mic SD / VDD | GPIO8 / 3V3 |
| Mic GND / L/R | GND; ảnh thể hiện L/R nối GND |
| Ampli BCLK / LRC | GPIO2 / GPIO1, dùng chung clock với mic |
| Ampli DIN | GPIO3 |
| Ampli VIN / GND | Đường nguồn nối chân 5V của board / GND |
| Ampli GAIN / SD | Không thấy dây ngoài trong ảnh; cần kiểm tra cấu hình sẵn của module |
| Loa | Ảnh chưa thể hiện loa; nếu dùng MAX98357A, nối loa giữa OUT+ và OUT−, không nối một đầu ra xuống GND |
| Pin dương | Đi qua công tắc tới chân 5V board và VIN ampli; một nhánh tới B+ mạch sạc |
| Pin âm và các ký hiệu GND | Cần thực sự nối chung bằng dây; ký hiệu trên ảnh không thay thế kết nối vật lý |
| Mạch USB-C phía trên | Chưa xác minh model, chức năng bảo vệ, nâng áp hoặc đường cấp tải |

Không có motor/servo trong ảnh này. Mic có hình dạng phù hợp loại I2S như INMP441; chưa xác nhận mã linh kiện thực tế. Firmware có driver SSD1306 nhưng chưa xác nhận độ phân giải/địa chỉ màn hình hoặc pin map trong binary.

## Điểm điện cần lưu ý

- Khi đang thử USB, tách nhánh pin/mạch sạc để tránh nối hai nguồn qua đường 5V khi chưa biết mạch có cách ly hay không.
- Đo 3V3–GND lúc khởi động và lúc gắn tải: kỳ vọng gần 3.3 V; ESP32-C3 có dải nguồn chip 3.0–3.6 V. Đồng hồ có thể bỏ sót sụt áp rất ngắn. Espressif khuyến nghị nguồn cho chip cấp được ít nhất 500 mA; ampli cần thêm dòng riêng tùy tải. Đây không phải giới hạn dòng của cổng USB hoặc regulator trên mọi board Super Mini.
- GPIO2/8/9 là chân strapping. Tuy nhiên GPIO8 thấp riêng lẻ không ngăn boot bình thường khi GPIO9 cao. GPIO8 cần cao khi GPIO9 thấp để vào download mode. Không quy lỗi cho GPIO8 chỉ vì mic dùng chân này; không tự thêm pull-up hoặc đổi dây khi chưa có log/source.
- GPIO20/21 còn là chân UART0 mặc định. Với OLED đang dùng hai chân này, ưu tiên serial qua cổng USB trực tiếp. Không đồng thời gắn USB-UART vào chúng cùng OLED.
- Nếu sau này chạy pin: sơ đồ đưa điện áp pin trực tiếp vào chân 5V; chưa thấy khối nâng áp. Không thể mặc định mạch sạc xuất 5 V. Phải xác minh pin, regulator của board và điện áp khi có tải trước khi dùng sơ đồ nguồn đó. Không nối pin Li-ion trực tiếp vào chân 3V3.

## Đọc log trên máy hiện tại

Máy đã có `esptool` 4.7.0 và `pyserial`; chưa cần cài dependency. Tại thời điểm kiểm tra không có `/dev/ttyACM*` hoặc `/dev/ttyUSB*` để kết nối.

Liệt kê cổng sau khi cắm board bằng cáp có truyền dữ liệu:

```bash
python3 -m serial.tools.list_ports
```

Thay `/dev/ttyACM0` bằng cổng thực tế, mở monitor rồi nhấn RESET một lần:

```bash
python3 -m serial.tools.miniterm --raw --dtr 0 --rts 0 /dev/ttyACM0 115200
```

Thoát bằng `Ctrl+]`. Nếu RESET làm USB ngắt/kết nối lại, mở lại cổng. Nếu không có log, firmware có thể xuất log qua UART thay vì USB; im lặng trên USB chưa chứng minh board không chạy. Muốn đọc UART cần tách OLED và xác minh cách kết nối adapter logic 3.3 V.

Gửi khoảng 30–60 dòng từ lúc reset, che thông tin mạng, địa chỉ thiết bị, mã kích hoạt và credential nếu có. Đối chiếu:

| Log | Diễn giải |
| --- | --- |
| `Brownout detector was triggered` hoặc reset khi tải tăng | Ưu tiên kiểm tra nguồn; reset đơn thuần chưa xác định nguyên nhân |
| `DOWNLOAD...` / `waiting for download` | Đang ở bootloader; kiểm tra nút BOOT/GPIO9, thả BOOT và reset |
| `SPI_FAST_FLASH_BOOT` | Đã chọn boot từ flash; vẫn cần đọc log phía sau |
| `ESP_ERROR_CHECK failed`, `abort`, lỗi I2C/SSD1306 | Có thể dừng trước bước Wi-Fi vì ngoại vi chưa khởi tạo được |
| Log Wi-Fi báo kết nối và nhận IP | Kết nối Wi-Fi nhà đã hoạt động; kiểm tra chế độ AP theo firmware |
| `invalid header` | Kiểm tra image/offset/flash; nguồn flash cũng có thể gây lỗi |

## Firmware đã kiểm tra

| Thuộc tính | Kết quả từ file |
| --- | --- |
| Kích thước | 3,541,630 bytes |
| SHA-256 | `3d9dc6d68cc0eebaacb05f20fb4cc49fda7802eda21400483f1d9fcf2a01cf98` |
| Chip ID | 5 — ESP32-C3 |
| Header flash | 4 MB, DIO, 80 MHz |
| Phiên bản chip tối thiểu trong header | v0.3; cần đối chiếu board thật trước khi nạp |
| Bootloader / partition table / factory app | `0x0` / `0x8000` / `0x10000` |
| NVS / assets | `0x9000` / `0x300000` |
| App | `xiaozhi` 2.4.0, ESP-IDF `v5.5.2-dirty` |
| Chuỗi board | `pocket-wall-e-c3`, `PocketWallEC3Board` |
| Driver màn hình | Có chuỗi SSD1306; chưa đủ để xác minh màn hình thực tế |
| Bootloader và app checksum/hash | Hợp lệ; không xác minh tính đúng đắn của toàn bộ assets hoặc nguồn gốc firmware |

## Nạp lại chỉ khi cần, sau khi lấy log và sao lưu

Các lệnh dưới dùng cú pháp esptool **4.7.0 đang có trên máy**. Chưa chạy với thiết bị. Đóng serial monitor trước khi dùng esptool. Tách ngoại vi và pin; dùng USB trực tiếp.

```bash
esptool --chip esp32c3 --port /dev/ttyACM0 chip_id
esptool --chip esp32c3 --port /dev/ttyACM0 flash_id
```

Nếu không tự vào download: giữ BOOT, nhấn/thả RESET rồi thả BOOT, chạy lại lệnh. Xác nhận chip/revision tương thích và flash ít nhất 4 MB. Nếu flash đúng 4 MB, sao lưu trước bằng tên file mới:

```bash
mkdir -p logs
esptool --chip esp32c3 --port /dev/ttyACM0 read_flash 0x0 0x400000 logs/pocket-before-flash.bin
```

Nếu flash có kích thước khác, cần điều chỉnh kích thước backup theo kết quả `flash_id`. Backup có thể chứa credential; giữ local, không commit/chia sẻ. `logs/` và `*.bin` đã được ignore.

**Chỉ thực hiện bước sau khi thực sự quyết định thay firmware:** file này là merged image, bắt đầu ở `0x0`, không nạp cả file ở `0x10000`. Lệnh sẽ ghi đè vùng NVS nằm trong image, có thể mất Wi-Fi/cấu hình/ghép nối đang lưu. Không cần `erase_flash` toàn chip trước.

```bash
esptool --chip esp32c3 --port /dev/ttyACM0 --baud 115200 write_flash 0x0 /home/nunu/Downloads/pocket-esp-ai.bin
```

Sau khi nạp, thả BOOT và reset. Quy trình AP, trang cấu hình và kích hoạt tài khoản phải theo log/màn hình hoặc tài liệu gốc của firmware; chưa xác minh nên không đoán SSID, mật khẩu, IP hay dịch vụ.

## Bước code tiếp theo

Trước khi thay firmware, cần log board trần và cấu hình đầu tiên gây lỗi. Nếu cần viết firmware chẩn đoán, trình plan riêng để người dùng duyệt: boot/reset logging, quét I2C với SDA21/SCL20, rồi kiểm tra audio từng bước; giữ bản backup để phục hồi. Muốn sửa ứng dụng Pocket gốc cần source tương ứng, `.bin` không thay thế source.

Không có ứng dụng web, build config, lint config hoặc Playwright trong repo hiện tại. Đợt này chỉ thêm tài liệu, không có firmware build/test, E2E hay video phần cứng. Kiểm tra local: metadata/checksum binary, cú pháp công cụ hiện có, liên kết nội bộ và whitespace diff. Kết luận robot hoạt động cần được xác nhận trên board thật.

## Tài liệu tham chiếu

- [Espressif: Boot mode selection](https://docs.espressif.com/projects/esptool/en/latest/esp32c3/advanced-topics/boot-mode-selection.html)
- [Espressif: Power supply, strapping, UART](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32c3/schematic-checklist.html)
- [Espressif: esptool flash/read/image commands](https://docs.espressif.com/projects/esptool/en/latest/esp32c3/esptool/basic-commands.html) — tài liệu mới dùng dấu gạch nối; máy đang có v4 dùng dấu gạch dưới như các lệnh trên.
- [Analog Devices: MAX98357A](https://www.analog.com/en/products/max98357a.html)
- [TDK: INMP441 datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/INMP441.pdf)
