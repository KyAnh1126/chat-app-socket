# chat-app-socket

1. server.cpp
- file xử lý các request từ client
- server nhận tin nhắn từ client, và gửi tin nhắn đó tới tất cả client trong phòng chat (ngoại trừ client gửi tin nhắn)
- server được chia thành nhiều luồng, mỗi luồng xử lý một client
- các signal được gọi để khi user nhận 'quit' từ bàn phím hoặc nhấn CTRL_C, server đóng tất cả socket đang được kết nối lẫn socket của server
- semaphore được sử dụng để giới hạn số lượng client được chat trong phòng
- mutex được sử dụng để đồng bộ hóa tiến trình (tránh các luồng tranh chấp tài nguyên)
- socket được sử dụng với kết nối TCP (địa chỉ IP: 127.0.0.1 - mạng cục bộ, cổng 8080) để đảm bảo dữ liệu được kiểm tra và gửi lại nếu lỗi

2. client.cpp
- file xử lý việc gửi request từ client tới server
- socket được sử dụng, client cần biết địa chỉ IP và port của server để gửi request tới server 
- client được chia thành hai luồng là gửi và nhận tin nhắn
- các signal được gọi (giống server) để đóng socket đang được kết nối khi client nhập 'quit' hoặc nhấn CTRL_C
