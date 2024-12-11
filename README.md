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
![image](https://github.com/user-attachments/assets/558d056f-4df2-4b9a-b622-3d2a13fe60dc)

    + client 4 nếu tham vào phòng chat đã đủ 3 người sẽ phải đợi cho đến 1 trong 3 người rời khỏi phòng (do semaphore):
![image](https://github.com/user-attachments/assets/86615f5b-e069-48e2-8235-b8b7b8b65688)

- phía server hiển thị thông tin về client khi enter vào phòng chat:
    + ![image](https://github.com/user-attachments/assets/21d727b8-a52a-4edd-b016-36611db55a3c)

    + ![image](https://github.com/user-attachments/assets/3e4012a0-651d-45ec-8e96-21d0ad29dc60)


    

