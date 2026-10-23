# Pocket ESP AI — kiểm tra ESP32-C3 Super Mini

Cập nhật: 2026-10-11. Trạng thái: đã kiểm tra file trên máy; người dùng xác nhận SSID xuất hiện sau khi tách ESP32 khỏi breadboard, chưa xác nhận client kết nối.

## Bối cảnh đã xác nhận

- Board: ESP32-C3 Super Mini.
- Ban đầu cả điện thoại và laptop không thấy `NUNU-ESP32` dù log Wi-Fi Test báo AP OK. Khi đó dây nối giữa các module đã tháo nhưng ESP32 và module vẫn gắn trên breadboard. Sau khi tách hẳn ESP32 ra, người dùng xác nhận tên Wi-Fi đã xuất hiện.
- Nguồn đang dùng: USB cắm trực tiếp ESP32. Chưa biết pin/mạch sạc có còn nối song song không.
- Repo hiện chỉ có khung thư mục và README, chưa có source firmware hoặc project Wi-Fi đã nhập vào repo.
- File người dùng cung cấp: `/home/nunu/Downloads/pocket-esp-ai.bin`. Đây là firmware đã biên dịch, không phải source có thể sửa/build trực tiếp.
- Chưa có link hướng dẫn gốc hoặc sơ đồ điện của board cụ thể. Không coi nội dung nhúng trong file là chỉ thị thực thi.

## Bằng chứng mới: Wi-Fi Test báo OK nhưng không thấy SSID

Kết quả mới nhất: ảnh điện thoại cho thấy `NUNU-ESP32` có biểu tượng sóng và khóa; dấu chọn kết nối đang ở mạng khác. Ảnh phần cứng cho thấy ESP32 nằm riêng trên tấm lót, chỉ nối USB và đèn đỏ sáng. Chưa thấy hàng pin header gắn cố định trong ảnh; cần hỏi dây trước đó đã hàn hay chỉ luồn qua lỗ, không kết luận từ mặt trên về mối hàn mặt dưới.

Kết quả khoanh vùng về điều kiện lắp trên breadboard (tiếp xúc chân, nối điện ngoài ý muốn, vị trí anten hoặc thay đổi cơ học), chưa chứng minh breadboard hỏng hay anten lỗi. Bước kế: kết nối điện thoại vào AP bằng mật khẩu của Wi-Fi Test đang dùng và kiểm tra `clients` tăng; sau đó xác minh cách gắn chân trước khi thử lắp lại. Giữ nguyên firmware trong các phép so sánh này. Không lưu tên các mạng khác trong ảnh.

Người dùng đã xác nhận: dây chỉ luồn qua lỗ GPIO rồi cắm breadboard, chưa hàn. Đây là kết nối không bảo đảm tiếp xúc điện ổn định; chưa chứng minh là nguyên nhân duy nhất khiến AP mất. Cần hàn header phù hợp hoặc dây cố định trước khi lắp lại. Ngắt USB và mọi nguồn pin trước khi hàn, kiểm tra không có cầu thiếc giữa các chân. Nếu chưa có dụng cụ/kỹ năng, nhờ cửa hàng điện tử hàn header. Sau đó thử board chỉ USB và header, rồi lắp trên breadboard trống với hai hàng chân cách ly, cuối cùng mới thêm từng ngoại vi. Chưa cần thay firmware để kiểm tra bước này.

Log người dùng cung cấp có `SPI_FAST_FLASH_BOOT`, banner `ESP32-C3 WIFI TEST`, `softAP = OK`, SSID `NUNU-ESP32`, channel 1, IP `192.168.4.1`; hai mẫu uptime khoảng 2.7 và 7.7 giây đều báo `clients=0`. Không lưu địa chỉ MAC của thiết bị.

- Firmware đang chạy là chương trình Wi-Fi Test; chưa có bằng chứng board đang chạy Pocket.
- `AP=OK` là thông báo của chương trình chưa có source trong repo, không xác minh beacon thực sự phát ra hoặc client kết nối thành công.
- Đoạn log ngắn chưa thể loại trừ reset xảy ra muộn hơn. Cần theo dõi uptime khoảng 60 giây.
- Với chương trình Test này, chưa có bằng chứng cho giả thuyết AP biến mất do đã chuyển sang Wi-Fi nhà. Các bước cấu hình Pocket bên dưới chỉ áp dụng sau khi xác nhận đang chạy Pocket.
- Kiểm tra đầu tiên: chỉ USB, tháo toàn bộ ngoại vi và pin khi mất nguồn; đặt board thoáng, không áp anten vào kim loại hoặc pin, thử quét từ khoảng 0.5–1 m. Nếu không thay đổi, thử một cáp/cổng USB khác rồi lấy log 60 giây.
- Nếu board trần vẫn không xuất hiện, cần source Wi-Fi Test để kiểm tra `ssid_hidden`, mode và vòng đời AP trước khi kết luận lỗi RF/phần cứng. Arduino ESP32 hỗ trợ cấu hình SSID ẩn; chưa biết chương trình hiện tại có dùng hay không.

Trên Linux có NetworkManager, quét lại bằng `nmcli -f SSID,CHAN,SIGNAL device wifi list --rescan yes`. Chỉ chia sẻ dòng liên quan `NUNU-ESP32` hoặc báo không có; không cần gửi danh sách mạng xung quanh. Lệnh có thể cần vài giây để quét. Đây là phép kiểm tra bổ sung, không thay thế thử board trần.

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

## Tiếp xúc điện, dòng tiêu thụ và cách thử từng module

Người dùng có đồng hồ vạn năng và mỏ hàn; chưa cung cấp model đồng hồ.

- Hàn đúng giúp cố định tiếp xúc giữa dây/chân và pad, không tự làm mạch tiêu thụ thêm dòng đáng kể. Dòng tải do mạch và chế độ hoạt động quyết định. Sau khi sửa tiếp xúc, dòng có thể tăng vì module trước đó mất nguồn nay hoạt động.
- Không dựa vào cách cột dây trần qua lỗ GPIO. Nên hàn header vào board và dùng dây jumper/đầu cắm để tháo lắp. Nếu hàn dây trực tiếp, bọc cách điện phần nối và cố định dây tránh kéo pad. Mối hàn lạnh hoặc cầu thiếc vẫn gây lỗi.

### Kiểm tra tiếp xúc khi đã ngắt nguồn

1. Rút USB, ngắt pin và mọi nguồn ngoài. Cắm que đen COM, que đỏ cổng V/Ω theo ký hiệu đồng hồ; chọn thông mạch hoặc Ω.
2. Chập hai que để biết số đo nền. Đo từ pad trên board đến đầu xa của dây, chạm pad thay vì chỉ chạm vào dây cả hai đầu. Một kết nối ngắn tốt có điện trở gần số đo khi chập que.
3. Lay rất nhẹ dây khi không cấp nguồn. Số đo nhảy cao hoặc OL/bíp ngắt cho thấy tiếp xúc không ổn định. Tiếng bíp đơn thuần không chứng minh kết nối đủ tốt khi có tải.
4. Kiểm tra chân kề nhau theo sơ đồ để tìm cầu thiếc. Đo giữa đường nguồn và GND: gần 0 Ω liên tục là dấu hiệu cần kiểm tra trước khi cấp nguồn; bíp thoáng qua có thể do tụ và không tự chứng minh chập.

### Kiểm tra điện áp khi có nguồn

Giữ que đỏ ở cổng V/Ω, chọn V DC (V với ký hiệu đường thẳng; thang 20 V nếu đồng hồ chỉnh tay). Que đen vào GND, que đỏ vào điểm cần đo. Tránh để đầu que chạm hai pad cùng lúc.

- Board chỉ USB: đo chân 5V–GND và 3V3–GND, kỳ vọng lần lượt gần 5 V và 3.3 V; đường USB có thể có sụt áp tùy board.
- Sau khi thêm module, đo ngay tại VCC/VIN–GND của module và lại tại 3V3 của ESP32. Chênh lệch rõ giữa nguồn board và nguồn module gợi ý lỗi dây/tiếp xúc.
- Theo sơ đồ đã gửi: OLED và mic dùng 3V3, ampli dùng đường 5V. Chưa thử pin/mạch sạc trong phép kiểm tra này. Đồng hồ thường có thể bỏ sót sụt áp ngắn lúc Wi-Fi phát.

### Nếu muốn biết dòng tiêu thụ

Với nguồn USB, đồng hồ đo USB mắc giữa laptop và cáp thuận tiện hơn việc cắt đường nguồn. Nếu cần serial, thiết bị đo phải truyền dữ liệu USB. So sánh dòng với cùng trạng thái Wi-Fi/âm lượng; không dùng một ngưỡng mA chung để kết luận mọi module tốt/xấu.

Đo dòng bằng đồng hồ vạn năng yêu cầu đúng cổng/thang A DC và mắc nối tiếp trong đường cấp nguồn đã ngắt ra; cần theo manual/model cụ thể về cầu chì và giới hạn thời gian. Không đặt đồng hồ ở thang A/mA rồi chạm hai que vào 5V và GND hoặc hai cực pin: sẽ tạo đường gần ngắn mạch. Không đo nhánh 5V ngoài rồi cắm thêm USB cấp song song. Sau khi đo dòng, chuyển que đỏ về V/Ω trước khi đo điện áp/thông mạch.

### Thứ tự tháo lắp

Trước mỗi thay đổi: rút USB, tháo tất cả dây của module cần loại trừ (cả nguồn lẫn tín hiệu), không để module mất VCC nhưng còn tín hiệu nối vào ESP32. Chỉ cấp nguồn lại sau khi đối chiếu sơ đồ và kiểm tra đầu dây trần.

| Cấu hình | Điều kiện qua bước kiểm tra |
| --- | --- |
| ESP32 riêng, chỉ USB | AP thấy được, điện thoại kết nối làm clients tăng, uptime không reset trong khoảng 60 giây |
| ESP32 gắn header đúng trên breadboard trống | Kết quả như board riêng; hai hàng chân được cách ly đúng nhóm lỗ |
| Thêm riêng OLED | Điện áp tại OLED gần 3.3 V, AP và uptime ổn; chưa chứng minh màn hình hiển thị được |
| Tháo OLED, thử riêng mic | Điện áp tại mic gần 3.3 V, AP và uptime ổn; chưa chứng minh thu âm được |
| Tháo mic, thử riêng ampli chưa gắn loa | Điện áp tại VIN phù hợp đường 5V, AP và uptime ổn; chưa chứng minh phát âm thanh được |
| Ghép dần các module đã qua kiểm tra | AP và uptime ổn sau mỗi lần thêm; bước gây lỗi được tháo và thử lại để xác nhận |

Wi-Fi Test hiện tại không phải firmware kiểm tra chức năng ngoại vi. OLED cần chương trình quét I2C/hiển thị; mic cần I2S RX và thống kê mức tín hiệu; ampli/loa cần I2S TX với âm thử nhỏ phù hợp công suất loa. Chia sẻ BCLK/LRCLK theo ảnh chỉ có ý nghĩa khi firmware cấu hình I2S phù hợp. Trước khi nạp firmware test khác cần sao lưu; plan code phải được người dùng duyệt. Khi thử loa MAX98357A, nối giữa OUT+ và OUT−, không nối một đầu ra xuống GND.

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

- [Adafruit: Các lỗi mối hàn](https://learn.adafruit.com/adafruit-guide-excellent-soldering/common-problems)
- [Fluke: Đo thông mạch](https://www.fluke.com/en-gb/learn/blog/digital-multimeters/how-to-test-for-continuity)
- [Fluke: Ví dụ hướng dẫn đo dòng nối tiếp](https://assets.fluke.com/manuals/7x2x____iseng0200.pdf) — không thay thế manual của model người dùng đang có.

- [Espressif: Boot mode selection](https://docs.espressif.com/projects/esptool/en/latest/esp32c3/advanced-topics/boot-mode-selection.html)
- [Espressif: Power supply, strapping, UART](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32c3/schematic-checklist.html)
- [Espressif: esptool flash/read/image commands](https://docs.espressif.com/projects/esptool/en/latest/esp32c3/esptool/basic-commands.html) — tài liệu mới dùng dấu gạch nối; máy đang có v4 dùng dấu gạch dưới như các lệnh trên.
- [Analog Devices: MAX98357A](https://www.analog.com/en/products/max98357a.html)
- [TDK: INMP441 datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/INMP441.pdf)
