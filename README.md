# Community User & Server

基于 Qt 的 C/S 架构即时通讯应用，包含 TCP 通信服务端和客户端。

## 项目结构

```
communityUserAndServer/
├── re_communication_server/   # 服务端
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── widget.h / .cpp / .ui   # 主窗口 + TCP 服务器
│   └── .gitignore
├── re_communication_user/     # 客户端
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── widget.h / .cpp / .ui   # 连接界面
│   ├── log.h / .cpp / .ui      # 登录界面
│   ├── register.h / .cpp / .ui # 注册界面
│   ├── use.h / .cpp / .ui      # 主聊天界面
│   └── .gitignore
└── README.md
```

## 功能

- **用户注册/登录** — 客户端注册新账号，登录已有账号
- **好友管理** — 添加好友、查看好友申请、接受/拒绝申请、删除好友
- **即时通讯** — 好友间发送消息，在线用户实时接收推送
- **聊天记录** — 服务端 SQLite 持久化存储，客户端拉取历史消息
- **在线状态** — 点击好友查看在线状态

## 技术栈

| 层 | 技术 |
|---|------|
| 语言 | C++17 |
| UI框架 | Qt 5/6 (Widgets) |
| 通信 | TCP Socket (QTcpServer / QTcpSocket) |
| 序列化 | JSON (QJsonDocument / QJsonObject) |
| 数据库 | SQLite (QSqlDatabase) |
| 构建 | CMake 3.16+ |

## 通信协议

客户端与服务端通过 JSON 文本协议通信，每条消息包含 `cmd` 字段标识操作类型：

| cmd | 方向 | 说明 |
|-----|------|------|
| `register` | Client → Server | 注册新用户 |
| `login` | Client → Server | 用户登录 |
| `add_friend` | Client → Server | 发送好友申请 |
| `get_pending_requests` | Client → Server | 查看好友申请列表 |
| `accept_friend` | Client → Server | 接受好友申请 |
| `get_friends` | Client → Server | 获取好友列表 |
| `delete_friend` | Client → Server | 删除好友 |
| `send_message` | Client → Server | 发送消息 |
| `get_messages` | Client → Server | 拉取聊天记录 |
| `new_message` | Server → Client | 实时消息推送 |

## 构建与运行

### 依赖

- Qt 5.15+ 或 Qt 6.x (Widgets, Network, Sql 模块)
- CMake 3.16+
- 支持 C++17 的编译器 (MSVC 2019+, GCC 8+, Clang 10+)

### 编译

```bash
# 服务端
cd re_communication_server
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build build

# 客户端
cd re_communication_user
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/Qt
cmake --build build
```

### 运行

1. 先启动 **服务端** (`re_communication_server`)，默认监听 `0.0.0.0:8000`
2. 启动 **客户端** (`re_communication_user`)，输入服务器 IP 和端口 (8000) 连接
3. 注册新账号或登录已有账号
4. 通过好友 ID 添加好友，开始聊天

## 数据库表结构

服务端使用 SQLite (`chat.db`) 存储数据：

| 表 | 字段 | 说明 |
|----|------|------|
| `users` | username, password | 用户账号 |
| `friends` | username, friendname | 好友关系 (双向) |
| `pending_requests` | from_user, to_user | 待处理好友申请 |
| `messages` | id, from_user, to_user, content | 聊天消息记录 |

## License

MIT
