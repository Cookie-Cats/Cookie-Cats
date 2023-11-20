#include "webpages.h"

using namespace std;

// 读取网页
// 如果可以被找到，则返回网页内容
// 否则返回值为空
String ICACHE_FLASH_ATTR webPages(String &page) {
  if (page == "/index.html") {
    String index = F(R"=====(
<!DOCTYPE html>
<html lang="zh">
<head>
 <meta charset="UTF-8">
 <meta name="viewport" content="width=device-width, initial-scale=1.0">
 <title>Cookie-Cats</title>
 <style>
  #Cookie-Cats {
padding: 20px;
height: 10px;
  }

  .cat {
position: relative;
height: 80px;
width: 80px;
background-color: #ffc400;
border-radius: 10px 10px 50%/ 100% 100% 0px 0px;

.ear {
 position: absolute;
 top: -10px;
 height: 36px;
 width: 25px;
 border-top-left-radius: 6px;
 /* background-color: #5d5d5e; */
}

.left-ear {
 left: 5px;
 transform: rotate(6deg) skewY(48deg);
 background-color: #cf9ec2;
 border: #000000;
}

.right-ear {
 right: 4px;
 transform: rotate(33deg) skewY(48deg);
 background-color: #000000;
}

.eye {
 position: absolute;
 top: 24px;
 height: 11px;
 width: 13px;
 border-radius: 50%;;
 background-color: #fff;
}

.left-eye {
 left: 22px;
}

.right-eye {
 right: 22px;
}

.mouth {
 position: absolute;
 top: 45px;
 left: 50%;
 transform: translateX(-50%);
 height: 2px;
 width: 8px;
 background-color: #fff;
}

.tail {
 position: absolute;
 bottom: -0px;
 left: -16px;
 width: 60px;
 height: 16px;
 border: 6px solid #000000;
 border-radius: 30%;
 border-top: none;
}
  }

  * {
margin: 0;
padding: 0;
  }

  html {
height: 100%;
  }

  body {
height: 100%;
  }

  .container {
padding: 50px 0px;
height: 100%;
background-image: linear-gradient(to right, #003377, #c3a3b4);
  }

  .login-wrapper {
background-color: #fff;
width: 558px;
/* height: 588px; */
border-radius: 15px;
padding: 0 50px;
position: relative;
left: 50%;
/* top: 50%; */
/* margin-left: -50%; */
transform: translateX(-50%);
  }

  .header {
font-size: 38px;
font-weight: bold;
text-align: center;
line-height: 200px;
  }

  .input-item {
box-sizing: border-box;
display: block;
width: 100%;
margin-bottom: 20px;
border: 0;
padding: 10px;
border-bottom: 1px solid rgb(128, 125, 125);
font-size: 15px;
outline: none;
  }

  .input-item::placeholder {
text-transform: uppercase;
  }

  .btn {
box-sizing: border-box;
text-align: center;
padding: 10px;
width: 100%;
margin-top: 40px;
background-image: linear-gradient(to right, #003377, #c3a3b4);
color: #fff;
  }

  .msg {
text-align: center;
line-height: 88px;
  }

  a {
text-decoration-line: none;
color: #ABB1C7;
  }

  .custom-select {
padding-left: 5px;
box-sizing: border-box;
width: 100%;
/* padding: 10px; */

background: #fff;
height: 38px;
border: 0;
margin-bottom: 20px;
font-size: 15px;
border-bottom: 1px solid rgb(128, 125, 125);
  }

  .input-item-box {
display: flex;

  }

  .input-item-box .custom-select-ip {
background: #fff;
height: 38px;
border: 0;
width: 100%;
margin-bottom: 20px;
font-size: 15px;
border-bottom: 1px solid rgb(128, 125, 125);
  }

  .input-item-box .input-item-ip {
box-sizing: border-box;
display: block;
width: 100%;
margin-bottom: 20px;
border: 0;
padding: 10px;
border-bottom: 1px solid rgb(128, 125, 125);
font-size: 15px;
outline: none;
margin-left: 10px;
  }

  .input-item-box .input-item-label {
box-sizing: border-box;
display: block;
width: 83px;
min-width: 83px;
margin-bottom: 20px;
border: 0;
font-size: 12px;
border-bottom: 1px solid rgb(128, 125, 125);
outline: none;
display: flex;
padding-left: 10px;
align-items: center;
  }

  .form-container {
display: flex;
  }

  .form-item-box {
flex: 1;
/* width: 50%; */
  }
 </style>
</head>
<body>
<div class="container">
 <div id="Cookie-Cats">
  <div class="cat">
<div class="ear left-ear"></div>
<div class="ear right-ear"></div>
<div class="eye left-eye"></div>
<div class="eye right-eye"></div>
<div class="mouth"></div>
<div class="tail"></div>
  </div>
 </div>
 <div class="login-wrapper">
  <div class="header">欢迎领养饼干喵~</div>
  <div class="form-container">
<form action="" method="post" onsubmit="submitForm()">
 <div class="form-item-box" style="margin-right: 10px;">
  <div class="form-wrapper">
<select class="custom-select" name="school" id="school">
 <option value="" selected>选择学校</option>
 <option value="ChinaPharmaceuticalUniversityDormitory">中国药科大学宿舍网</option>
</select>
<select class="custom-select" name="carrier" id="carrier">
 <option value="" selected>选择供应商</option>
 <option value="ChinaMobile">中国移动</option>
 <option value="ChinaUnicom">中国联通</option>
 <option value="ChinaTelecom">中国电信</option>
</select>
  </div>
  <div class="form-wrapper">
<input type="text" name="username" id="username" placeholder="用户名" class="input-item"/>
<input type="password" name="password" id="password" placeholder="密码" class="input-item"/>
  </div>
  <div class="input-item-box">
<div class="input-item-label">IP获取方式：</div>
<select class="custom-select-ip" name="IP_Obtain_Method" onchange="handleChangeIpType()"
  id="selectIpType">
 <option value="unnecessary" selected>无需获取</option>
 <option value="manual">手动输入</option>
 <option value="meow">meow</option>
</select>
  </div>
 </div>
</form>

<div class="form-item-box">
 <input type="text" name="WiFi_SSID" id="WiFi_SSID" placeholder="认证路由SSID" class="input-item"/>
 <input type="password" name="WiFi_PASSWORD" id="WiFi_PASSWORD" placeholder="认证路由密码"
  class="input-item"/>
 <input type="text" name="Cookie_Cat_SSID" id="Cookie_Cat_SSID" placeholder="Cookie-Cats无线名"
  class="input-item"/>
 <input type="password" name="Cookie_Cat_PASSWORD" id="Cookie_Cat_PASSWORD"
  placeholder="Cookie-Cats无线密码" class="input-item"/>
 <input type="hidden" name="IP_Obtain_Method" id="hiddenIpMethod">
 <div id="selectIpByInput"></div>
</div>
  </div>

  <div class="btn" onclick="handleSubmit()">
<input type="submit" value="我输入好啦~" style="background-color: transparent;color: white;border: none;"/>
  </div>

  <div class="msg">
平台使用有疑问?<a href="https://github.com/Cookie-Cats/Cookie-Cats/issues">联系我们</a>
  </div>

 </div>
</div>
<script>
 function handleChangeIpType() {
  const el = document.querySelector('#selectIpType')
  const input = document.querySelector('#selectIpByInput')
  const hidden = document.querySelector('#hiddenIpMethod')
  if (el.value === 'unnecessary') {
input.innerHTML = ''
hidden.value = 'unnecessary' // 提交内容为空
  }
  if (el.value === 'manual') {
input.innerHTML = '<input type="text" name="IP_Obtain_Method_Content" id="IP_Obtain_Method_Content" placeholder="请输入ip" class="input-item" />' // 在div中添加文本框
hidden.value = 'manual' // 设置隐藏input的值为manual
  }
  if (el.value === 'meow') {
input.innerHTML = '<input type="text" name="IP_Obtain_Method_Content" id="IP_Obtain_Method_Content" placeholder="请输入url" class="input-item" />' // 在div中添加文本框
hidden.value = 'meow' // 设置隐藏input的值为meow
  }
 }

 async function resetConfig() {
  try {
// 填完整的url
const response = await window.fetch('device/restart', {
 headers: {
  'Content-Type': 'application/json;charset=utf-8'
 }
});
if (!(response.status >= 200 && response.status < 300)) {
 throw new Error(`网络异常, statusCode(${response.status})`);
}
const json = await response.json();
if (json.success) {
 alert('重启成功')
} else {
 alert(json.error)
}
  } catch (e) {
alert('服务器异常')
console.log(e)
  }
 }

 async function postConfig(body) {
  try {
// 填完整的url
const response = await window.fetch('config/save', {
 method: 'POST',
 body: JSON.stringify(body),
 headers: {
  'Content-Type': 'application/json;charset=utf-8'
 }
});
if (!(response.status >= 200 && response.status < 300)) {
 throw new Error(`网络异常, statusCode(${response.status})`);
}
const json = await response.json();
if (json.success) {
 alert('饼干猫记住啦，我将会在3秒后重启以载入配置！')
 setTimeout(() => {
  resetConfig()
 }, 3000)
} else {
 alert(json.error)
}
  } catch (e) {
alert('服务器异常')
console.log(e)
  }
 };

 function handleSubmit() { //提交按钮事件
  const obj = {
 allowOTA: "true",
}
  ;["Cookie_Cat_SSID", "Cookie_Cat_PASSWORD", "WiFi_SSID", "WiFi_PASSWORD", "username", "password", "carrier", "school"].forEach(item => {
const element = document.getElementById(item)
obj[item] = element.value
  });
  const el = document.querySelector('#selectIpType')

  if (el.value === 'manual') {
obj.IP_Obtain_Method = {
 manual: document.querySelector('#IP_Obtain_Method_Content').value
}
  }
  if (el.value === 'meow') {
obj.IP_Obtain_Method = {
 meow: document.querySelector('#IP_Obtain_Method_Content').value
}
  }
  postConfig(obj)
 }
</script>
</body>
</html>
    )=====");
    return index;
  } else {
    String noFound = "";
    return noFound;
  }
}
