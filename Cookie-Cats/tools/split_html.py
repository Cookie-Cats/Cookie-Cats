#!/usr/bin/env/python3
import html

content = r"""#Cookie-Cats{padding:20px;height:10px}.cat{position:relative;height:80px;width:80px;background-color:#ffc400;border-radius:10px 10px 50%/ 100% 100% 0 0;.ear{position:absolute;top:-10px;height:36px;width:25px;border-top-left-radius:6px}.left-ear{left:5px;transform:rotate(6deg) skewY(48deg);background-color:#cf9ec2;border:#000}.right-ear{right:4px;transform:rotate(33deg) skewY(48deg);background-color:#000}.eye{position:absolute;top:24px;height:11px;width:13px;border-radius:50%;background-color:#fff}.left-eye{left:22px}.right-eye{right:22px}.mouth{position:absolute;top:45px;left:50%;transform:translateX(-50%);height:2px;width:8px;background-color:#fff}.tail{position:absolute;bottom:-0px;left:-16px;width:60px;height:16px;border:6px solid #000;border-radius:30%;border-top:0}}*{margin:0;padding:0}html{height:100%}body{height:100%}.container{padding:50px 0;height:100%;background-image:linear-gradient(to right,#037,#c3a3b4)}.login-wrapper{background-color:#fff;width:558px;border-radius:15px;padding:0 50px;position:relative;left:50%;transform:translateX(-50%)}.header{font-size:38px;font-weight:bold;text-align:center;line-height:200px}.input-item{box-sizing:border-box;display:block;width:100%;margin-bottom:20px;border:0;padding:10px;border-bottom:1px solid #807d7d;font-size:15px;outline:0}.input-item::placeholder{text-transform:uppercase}.btn{box-sizing:border-box;text-align:center;padding:10px;width:100%;margin-top:40px;background-image:linear-gradient(to right,#037,#c3a3b4);color:#fff}.msg{text-align:center;line-height:88px}a{text-decoration-line:none;color:#abb1c7}.custom-select{padding-left:5px;box-sizing:border-box;width:100%;background:#fff;height:38px;border:0;margin-bottom:20px;font-size:15px;border-bottom:1px solid #807d7d}.input-item-box{display:flex}.input-item-box .custom-select-ip{background:#fff;height:38px;border:0;width:100%;margin-bottom:20px;font-size:15px;border-bottom:1px solid #807d7d}.input-item-box .input-item-ip{box-sizing:border-box;display:block;width:100%;margin-bottom:20px;border:0;padding:10px;border-bottom:1px solid #807d7d;font-size:15px;outline:0;margin-left:10px}.input-item-box .input-item-label{box-sizing:border-box;display:block;width:83px;min-width:83px;margin-bottom:20px;border:0;font-size:12px;border-bottom:1px solid #807d7d;outline:0;display:flex;padding-left:10px;align-items:center}.form-container{display:flex}.form-item-box{flex:1}"""

# content = html.escape(content)

# 将 " 转换为 \"
# content = content.replace('"', '\\"')

# 将 ' 转换为 \'
# content = content.replace("'", "\\'")

result = []
for index in range(0, len(content), 475):
    # result.append('"' + content[index:index + 475] + '\\n"')
    result.append(content[index:index + 475])

print('F(', end="")
for i in range(len(result)):
    print(result[i])
print(');')