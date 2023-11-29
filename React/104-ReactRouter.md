##  ReactRouter6.0 安装使用   

### 介绍  
官方英文文档：https://reactrouter.com/en/main/start/overview
中文文档：http://www.reactrouter.cn/docs/getting-started/tutorial

### 安装 
```shell
# npm 安装
npm install react-router-dom
# yarn 安装
yarn add react-router-dom
# pnpm 安装
pnpm add react-router-dom
```

### 路由组件

#### BrowserRouter
```js
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom'
// 使用组件包裹
<BrowserRouter>
  <App />
</BrowserRouter>
```

#### Routes定义路由
```js
<Routes>
  <Route path='/' element={<App />}></Route>
  <Route path='/demo' element={<React />}></Route>
</Routes>
```

#### 组件定义
```js
function React() {
  return <h2>欢迎学习React课程</h2>
}
```



