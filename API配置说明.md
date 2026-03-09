# 硅基流动API配置示例

## 环境变量配置

### Windows系统

1. 打开命令提示符或PowerShell
2. 运行以下命令（替换为你的实际API密钥）：
```powershell
[Environment]::SetEnvironmentVariable("SILICONFLOW_API_KEY", "你的API密钥", "User")
```

3. 重启终端或重新登录系统使环境变量生效

### Linux/Mac系统

在 `~/.bashrc` 或 `~/.zshrc` 文件中添加：
```bash
export SILICONFLOW_API_KEY="你的API密钥"
```

然后运行 `source ~/.bashrc` 或 `source ~/.zshrc` 使配置生效

## 代码配置（不推荐，仅用于测试）

在 `aiassistant.cpp` 文件的第24行附近修改：

```cpp
AIAssistant::AIAssistant(QObject* parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    // 硅基流动API密钥，需要在 https://cloud.siliconflow.cn/ 获取
    // 你可以将API密钥设置为环境变量或在代码中配置
    m_apiKey = qgetenv("SILICONFLOW_API_KEY");
    if (m_apiKey.isEmpty()) {
        // 默认API密钥（请替换为你自己的）
        m_apiKey = "sk-xxxxxxxxxxxxxxxxxxxxxxxx"; // 替换为你的实际API密钥
    }
}
```

## 验证配置

配置完成后，可以运行以下命令验证：

### Windows
```powershell
echo $env:SILICONFLOW_API_KEY
```

### Linux/Mac
```bash
echo $SILICONFLOW_API_KEY
```

## 安全建议

1. **不要将API密钥提交到Git仓库**
   - 确保 `.gitignore` 文件包含以下内容：
   ```
   # API密钥配置
   *.key
   *.env
   ```

2. **使用环境变量**
   - 环境变量比硬编码在代码中更安全

3. **定期更换API密钥**
   - 定期在硅基流动控制台更换API密钥

4. **限制API访问**
   - 如果可能，限制API的访问IP范围

## 常见问题

### 1. 环境变量不生效

- 确保重启了终端或IDE
- Windows系统可能需要重新登录

### 2. API密钥格式错误

- API密钥通常以 "sk-" 开头
- 确保没有多余的空格或换行

### 3. 权限不足

- 确认API账户有足够的配额
- 检查API密钥是否有访问权限

## 获取API密钥

1. 访问：https://cloud.siliconflow.cn/
2. 注册并登录账号
3. 进入"API密钥"页面
4. 点击"创建新密钥"
5. 复制并保存你的API密钥
