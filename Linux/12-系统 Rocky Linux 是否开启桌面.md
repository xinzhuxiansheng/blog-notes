可以在不需要重装系统的情况下，通过以下命令随时调整。

### 1. 临时切换（不需要重启，立即生效）

如果您现在正在用图形界面，觉得卡，想立刻关掉它释放内存：

*   **从图形界面 -> 切换到纯命令行：**
    ```bash
    # 停止图形服务，进入多用户文本模式
    sudo systemctl isolate multi-user.target
    ```
    *执行后，图形界面会立刻消失，变成黑底白字的登录终端。资源瞬间释放。*

*   **从纯命令行 -> 切换回图形界面：**
    ```bash
    # 启动图形服务
    sudo systemctl isolate graphical.target
    ```
    *执行后，桌面环境会重新加载。*

---

### 2. 永久修改默认启动模式（重启后生效）

如果您希望这台虚拟机以后**默认开机不进桌面**（节省资源），只有偶尔需要时再手动开启：

*   **设置为默认开机进入“纯命令行模式”：**
    ```bash
    sudo systemctl set-default multi-user.target
    ```
    *(下次重启后，直接进黑框框，不加载桌面)*

*   **恢复为默认开机进入“图形界面模式”：**
    ```bash
    sudo systemctl set-default graphical.target
    ```

---

### 💡 最佳实践场景

我建议您采用以下**“省流”用法**：

1.  **日常状态**：将默认启动设置为 `multi-user.target`（纯命令行）。
    *   这样虚拟机启动极快，且只占用几百兆内存。
    *   您平时通过 SSH 连接去操作它，跑跑脚本、存存数据。

2.  **临时需求**：当您真的需要进桌面去测试浏览器或看图表时：
    *   在 SSH 里输入 `sudo systemctl isolate graphical.target`。
    *   去 VMware 窗口里操作图形界面。
    *   用完后，再输入 `sudo systemctl isolate multi-user.target` 关掉它。

这样既能保证系统长期运行的低消耗，又能随时响应您的可视化需求。