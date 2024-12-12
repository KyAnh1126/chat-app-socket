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

3. Kết quả chương trình:
- phía client gồm 4 client tham gia vào phòng chat cùng một lúc:
    + ví dụ về client 1:
![10](https://github.com/user-attachments/assets/53ee7d22-a90d-4cd0-83b9-e7d26c35f0bf)

    + client 4 nếu tham vào phòng chat đã đủ 3 người sẽ phải đợi cho đến 1 trong 3 người rời khỏi phòng (do semaphore):
![11](https://github.com/user-attachments/assets/e8692597-cb14-4095-ab39-af627fbcf057)

- phía server hiển thị thông tin về client khi enter vào phòng chat:
    + ![12](https://github.com/user-attachments/assets/1ae11685-0cb2-4280-88ee-8b7e85d14d5f)

    + ![13](https://github.com/user-attachments/assets/2044fa16-722f-4865-bf92-41b71b17ebfa)



    

